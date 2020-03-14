#include "imgprocessing.h"

// #include <opencv2/core/core_c.h>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/face.hpp>
#include <pthread.h>
#include <iostream>

#ifdef PRINT_EYE_ROI
	FILE *fp;
	int imageEyeIndex = 0;
#endif

#ifdef CHECK_EYE_DIRECTION
	FILE *fp2;
#endif

FACEFITTER_T fftr;
bool SSUCCESS = 0;
std::vector<std::vector<cv::Point2f>> SLANDMARKS;
cv::Mat SSCALED_IMG;
std::vector<cv::Rect> SFACE(1);
sem_t LANDMARK_READY, FIT_THIS;

unsigned char findThreshold(cv::Mat *frame) {
	// unsigned int avg = 0;
	unsigned char thresh = 0;

	for(int y = 0; y < frame->rows; y++){
		for(int x = 0; x < frame->cols; x++){
			unsigned char value = frame->at<unsigned char>(y,x);
			if (value < thresh)
				thresh = value;
			// avg += value;
		}
	}

	// avg = avg / (frame->rows * frame->cols);
	// thresh = (unsigned char) avg / 1.7;
	thresh = thresh * 0.5;

	return thresh;

}

int initCAReOCV(int height, int width, FACERECOG_T *fr) {
	/********* setup opencv */
	if(!fr->face_cascade.load("/installation/OpenCV-/share/opencv4/haarcascades/haarcascade_frontalface_alt.xml")) 
		return -1;

	// This creates a continues (1-D array) set of data to allocate the image
	fr->image.create(height, width, CV_8UC1);

	fr->height = height;
	fr->width = width;
	fr->scale = 1;	// no scale image to process
	// checks if the memory allocated for img are all together
	if(!fr->image.isContinuous())
		return -1;

	return 0;
}

void CAReOCV_plugImage2Recognizer(FACERECOG_T *fr, unsigned char **image) {
	*image = fr->image.ptr(0);
}


inline void oCV_checkFrameRate (struct timespec *t1) {
	static float CAReOCV_frames = 0;
	struct timespec t2;

	CAReOCV_frames++;

	clock_gettime(CLOCK_MONOTONIC, &t2);

	float d = (t2.tv_sec + t2.tv_nsec / 1000000000.0) - (t1->tv_sec + t1->tv_nsec / 1000000000.0);
	float fps = CAReOCV_frames / d;

	fprintf(stderr, "[OpenCV] framerate %.2f fps \n", fps);
}

void predictEyeContainer(cv::Rect *tmpFaces, cv::Rect *eyes, double a, int rows, int cols) {

	tmpFaces->x = eyes->x + (1 - a) * eyes->width / 2;
	tmpFaces->y = eyes->y + (1 - a) * eyes->height / 2;

	if (tmpFaces->x < 0) tmpFaces->x = 0;
	if (tmpFaces->y < 0) tmpFaces->y = 0;

	tmpFaces->width = eyes->width * a;
	tmpFaces->height = eyes->height * a;

	
	if ((tmpFaces->y + tmpFaces->height) > rows)
		tmpFaces->height = rows - tmpFaces->y;

	if ((tmpFaces->x + tmpFaces->width) > cols)
		tmpFaces->width = cols - tmpFaces->x;
}

void removeExtraLeftEye(std::vector<cv::Rect> *lefteye, cv::Rect *eyes) {
	unsigned max_left = 0;

	for (unsigned j = 1; j < lefteye->size(); ++j) {
		if (lefteye->at(j).x > lefteye->at(max_left).x)
			max_left = j; 
	}
	*eyes = lefteye->at(max_left);
}

void removeExtraRightEye(std::vector<cv::Rect> *localEye, cv::Rect *eyes) {
	unsigned max_left = 0;
	for (unsigned j = 1; j < localEye->size(); ++j) {
		if (localEye->at(j).x < localEye->at(max_left).x)
			max_left = j; 
	}
	*eyes = localEye->at(max_left);
}

inline void computeAbsolutePosition (unsigned char previous, cv::Rect *eyes, cv::Rect *faces, cv::Rect *tmpFaces) {
	if (!previous) {
		eyes->x += faces->x; 
		eyes->y += faces->y;
	} else {
		eyes->x += tmpFaces->x; 
		eyes->y += tmpFaces->y; 
	}
}

void imNormalization(cv::Mat *im) {
	for (int y = 0; y < im->rows; y++) {
		for (int x = 0; x < im->cols; x++) {
			unsigned char val = im->at<unsigned char>(y, x);
			if (val < 16) {
				im->at<unsigned char>(y, x) = 0;
			} else {
				im->at<unsigned char>(y, x) -= 16;
			}
			im->at<unsigned char>(y, x) = val * 1.164;
		}
	}
}

void adjustLandmarksIntoCurrentFace(std::vector<cv::Point2f> *newLM, cv::Rect *face, std::vector<cv::Point2f> *holdLM, cv::Rect *holdFace) {
	double rw = face->width / holdFace->width;
	double rh = face->height / holdFace->height;

	for (int i = 36; i <= 47; ++i) {
		newLM->at(i).x = face->x + (holdLM->at(i).x - holdFace->x) * rw;
		newLM->at(i).y = face->y + (holdLM->at(i).y - holdFace->y) * rh;
	}
}

void computeEyeBB(std::vector<cv::Point2f> *landmark, cv::Rect *eye, EYEINDEX_T *eyeI) {
	double minx, maxx, miny, maxy;
	minx = miny = 360;
	maxx = maxy = 0;

	for (unsigned int j = eyeI->i0; j <= eyeI->i1; j++) {
		if (landmark->at(j).x > maxx)
			maxx = landmark->at(j).x;

		if (landmark->at(j).y > maxy)
			maxy = landmark->at(j).y;

		if (landmark->at(j).x < minx)
			minx = landmark->at(j).x;

		if (landmark->at(j).y < miny)
			miny = landmark->at(j).y;
	}

	eye->x = (unsigned) minx; 
	eye->y = (unsigned) miny;
	eye->width = (unsigned) (maxx) - eye->x;
	eye->height = (unsigned) (maxy) - eye->y;

	if (eye->height == 0) eye->height = 1;
}

int computePupilPosition(cv::Mat *scaledImg, std::vector<cv::Point2f> *landmark, cv::Rect *eye, unsigned char *thresh, double *centroids, EYEINDEX_T *eyeI, double *posxy) {
	int pos_x, pos_y;
	double posd_x;
	// double posd_y;
	int positives;
	unsigned minVal = 255;

	double m1 = (landmark->at(eyeI->i0).y - landmark->at(eyeI->tl0).y) 
				/ (landmark->at(eyeI->i0).x - landmark->at(eyeI->tl0).x);
	double q1 = landmark->at(eyeI->tl0).y - m1 * (landmark->at(eyeI->tl0).x);

	double m2 = (landmark->at(eyeI->tl1).y - landmark->at(eyeI->c).y) 
				/ (landmark->at(eyeI->tl1).x - landmark->at(eyeI->c).x);
	double q2 = landmark->at(eyeI->c).y - m2 * (landmark->at(eyeI->c).x);

	pos_x = 0;
	pos_y = 0;
	positives = 0;

	for (int y = eye->y; y < eye->y + eye->height; y++) {
		for (int x = eye->x; x < eye->x + eye->width; x++) {
			int why1 = (int) (m1 * x + q1);
			int why2 = (int) (m2 * x + q2);
			
			if (y <= why1 || y <= why2) {
				#ifdef PRINT_EYE_ROI
					fprintf(fp, "255, ");
				#endif
			} else {
				unsigned char val = scaledImg->at<unsigned char>(y, x);
				if (val < *thresh) { // thresh0 = 25; 
					positives++;
					pos_x += x;
					pos_y += y;
				}
				if (val < minVal) {
					minVal = val;
				}
				#ifdef PRINT_EYE_ROI
					fprintf(fp, "%d, ", scaledImg->at<unsigned char>(y, x));
				#endif
			}
		}
		#ifdef PRINT_EYE_ROI
			fprintf(fp, "\n");
		#endif
	}
	*thresh = minVal + 25;

	#ifdef PRINT_EYE_ROI
		fprintf(fp, "];\n");
	#endif

	if (positives) {

		posd_x = (double) pos_x / (double) positives;
		// posd_y = (double) pos_y / (double) positives;

		// *centroids = (double) pos_x / (double) positives;
		*(centroids + 1) = (double) pos_y / (double) positives;

		*centroids = (posd_x - eye->x) / (double) eye->width;
		// *(centroids + 1) = (posd_y - eye->y) / (double) eye->height;
		return 0;

	}
	return 1;
}


void estimateCentroid(double *centroids, std::vector<cv::Rect> *eye, int rcL, int rcR) {
	// Initial vals
	static double Pk = 1.0;
	static double Xk = 0.5;
	// Covariance Matrix
	static double R11 = 0.0010;
	static double R22 = 0.0015;
	static double R12 = 0.0001;
	static double R21 = 0.0001;
	// Pre-computed vals
	static double R22_R21 = R22 - R21;
	static double R11_R12 = R11 - R12;

	if (rcR) 
		centroids[2] = centroids[0];
	if (rcL)
		centroids[0] = centroids[2];

	// Update
	double detM = Pk * (R22_R21 + R11_R12) + R11*R22 - R12*R21;
	double Gk0 = Pk * R22_R21 / detM;
	double Gk1 = Pk * R11_R12 / detM;
	Xk = Xk + Gk0 * (centroids[0] - Xk) + Gk1 * (centroids[2] - Xk);
	Pk = (1 - Gk0 - Gk1) * Pk;
	// Predict
	centroids[0] = centroids[2] = Xk;
	Pk = Pk + 0.0005;

	double posd_x = Xk * ((double) eye->at(0).width) + eye->at(0).x;

	eye->at(0).x = posd_x - eye->at(0).height / 2;
	eye->at(0).y = centroids[1] - eye->at(0).height / 2; // this is because there's no kalman for y axis
	centroids[1] = (centroids[1] - eye->at(0).y) / eye->at(0).height;
	eye->at(0).width = eye->at(0).height;

	posd_x = Xk * ((double) eye->at(1).width) + eye->at(1).x;
	eye->at(1).x = posd_x - eye->at(1).height / 2;
	eye->at(1).y = centroids[3] - eye->at(1).height / 2;
	centroids[3] = (centroids[3] - eye->at(1).y) / eye->at(1).height;

	eye->at(1).width = eye->at(1).height;

}


void *oCV_runRecognizerTask3(void *arg) {
	FACERECOG_T *fr = (FACERECOG_T *) arg;
	cv::Mat scaledImg, secondImg;
	std::vector<cv::Rect> faces;
	std::vector<cv::Rect> eyes(2);
	EYEINDEX_T eyeL, eyeR;
	cv::Rect tmpROI;
	cv::Rect holdFace;
	unsigned char thresh[2];
	unsigned char prevHCFace = 0;
	double centroids[4];
	double posxyL[2 * 13];
	double posxyR[2 * 13];
	bool success = 0;
    std::vector<std::vector<cv::Point2f>> landmarks;

    memset(posxyL, 0, 2 * 13 * sizeof(double));
    memset(posxyR, 0, 2 * 13 * sizeof(double));
	eyeL.i = 0;
	eyeL.i0 = 36;	// initial corner
	eyeL.tl0 = 37;	// top lid 0
	eyeL.tl1 = 38;	// top lid 1
	eyeL.c = 39;	// tear duct
	eyeL.i1 = 41;	// last point

	eyeR.i = 1;
	eyeR.i0 = 42;	// initial corner
	eyeR.tl0 = 43;	// top lid 0
	eyeR.tl1 = 44;	// top lid 1
	eyeR.c = 45;	// tead duct
	eyeR.i1 = 47;	// last point

	thresh[eyeL.i] = 25;
	thresh[eyeR.i] = 25;

	cv::Ptr<cv::face::Facemark> facemark = cv::face::FacemarkLBF::create();
	facemark->loadModel("data/lbfmodel.yaml");

	scaledImg.create(fr->height / fr->scale, fr->width / fr->scale, CV_8UC1);
	SSCALED_IMG.create(fr->height / fr->scale, fr->width / fr->scale, CV_8UC1);

	#ifdef PRINT_EYE_ROI
		unsigned imageEyeIndex = 0;
		fp = fopen("eyesframes.m", "w");
	#endif

	#ifdef CHECK_EYE_DIRECTION
		fp2 = fopen("eyesDirection.dat", "w");
	#endif

	#ifdef CHECK_ET_OVERALL
		struct timespec overall_begin;
		struct timespec overall_end;
	#endif

	#ifdef CHECK_ET_FIT_DETECTION
		struct timespec ftt_begin;
		struct timespec ftt_end;
	#endif

	#ifdef CHECK_ET_FACE_DETECTION
		struct timespec ft_begin;
		struct timespec ft_end;
		clock_gettime(CLOCK_MONOTONIC, &ft_begin);
	#endif

	#ifdef IMG_PROCESSING_CHECK_FRAMERATE
		struct timespec t1;
		clock_gettime(CLOCK_MONOTONIC, &t1);
	#endif

	while(fr->exit) {
		sem_wait(fr->InFrameReady);			// waits for the frame stream
		
		#ifdef CHECK_ET_OVERALL
			clock_gettime(CLOCK_MONOTONIC, &overall_begin);
		#endif

		#ifdef IMG_PROCESSING_CHECK_FRAMERATE
			oCV_checkFrameRate(&t1);
		#endif
		cv::resize(fr->image, scaledImg, scaledImg.size(), 0, 0, cv::INTER_NEAREST);


		#ifdef CHECK_ET_FACE_DETECTION
			clock_gettime(CLOCK_MONOTONIC, &ft_begin);
		#endif

		if (prevHCFace) {
			double scaledUp = 1.25;
			predictEyeContainer(&tmpROI, &faces[0], scaledUp, scaledImg.rows, scaledImg.cols);
			secondImg = scaledImg(tmpROI);
			fr->face_cascade.detectMultiScale(secondImg, faces, 1.4, 3, 0 , cv::Size(tmpROI.width * (0.75 / scaledUp), tmpROI.height * (0.75 / scaledUp)));

 		} else {
			tmpROI.x = 0;
			tmpROI.y = 0;
			fr->face_cascade.detectMultiScale(scaledImg, faces, 1.4, 3, 0, cv::Size(70, 70), cv::Size(150, 150));
		}

		#ifdef CHECK_ET_FACE_DETECTION
			clock_gettime(CLOCK_MONOTONIC, &ft_end);
			float fd = (ft_end.tv_sec + ft_end.tv_nsec / 1000000000.0) - (ft_begin.tv_sec + ft_begin.tv_nsec / 1000000000.0);
			printf("face_HC ET %f ms\n", fd * 1000);		
		#endif


		if (faces.size() > 0) {
			
			faces[0].x += tmpROI.x;
			faces[0].y += tmpROI.y;
			prevHCFace = 1;

			#ifdef CHECK_ET_FIT_DETECTION
				clock_gettime(CLOCK_MONOTONIC, &ftt_begin);
			#endif 
			success = facemark->fit(scaledImg, faces, landmarks);
			#ifdef CHECK_ET_FIT_DETECTION
				clock_gettime(CLOCK_MONOTONIC, &ftt_end);
				float tc2 = (ftt_end.tv_sec + ftt_end.tv_nsec / 1000000000.0) - (ftt_begin.tv_sec + ftt_begin.tv_nsec / 1000000000.0);
				printf("face_fit ET %f ms\n", tc2 * 1000);		
			#endif

			if(success) {
				if (landmarks[0].size() == 68) {
					computeEyeBB(&landmarks[0], &eyes[eyeL.i], &eyeL);	
					computeEyeBB(&landmarks[0], &eyes[eyeR.i], &eyeR);

					#ifdef PRINT_EYE_ROI
						fprintf(fp, "imgL%d = [", imageEyeIndex);
					#endif
					int rcL = computePupilPosition(&scaledImg, &landmarks[0], &eyes[eyeL.i],
						&thresh[eyeL.i], centroids, &eyeL, posxyL);
				
					#ifdef PRINT_EYE_ROI
						fprintf(fp, "imgR%d = [", imageEyeIndex);
						imageEyeIndex++;
					#endif
					int rcR = computePupilPosition(&scaledImg, &landmarks[0], &eyes[eyeR.i],
						&thresh[eyeR.i], centroids + 2, &eyeR, posxyR);
					

					if (!rcL || !rcR) 
						estimateCentroid(centroids, &eyes, rcL, rcR);

					#ifdef CHECK_EYE_DIRECTION
						fprintf(fp2, "%f %f %f %f\n", centroids[0], centroids[1], centroids[2], centroids[3]);
					#endif

					// for Graphics 
					sem_wait(&fr->detectionReady);
					fr->buffer->rect = eyes;
					fr->buffer->state = EYES_FOUND;
					sem_post(&fr->detectionReady);

					// for Fusion
					sem_wait(&fr->positionReady);
					if (eyes[eyeL.i].height == 1 || eyes[eyeR.i].height == 1)
						fr->bufferPos->state = EYES_FOUND_CLOSED;
					else {
						fr->bufferPos->size = 2;
						fr->bufferPos->state = EYES_FOUND;
						memcpy(fr->bufferPos->data, centroids, 4 * sizeof(double));
					}
					sem_post(&fr->positionReady);

				} else {
					// for graphics
					sem_wait(&fr->detectionReady);
					fr->buffer->state = EYES_NOT_FOUND;
					sem_post(&fr->detectionReady);

					// for Fusion
					sem_wait(&fr->positionReady);
					fr->bufferPos->state = EYES_NOT_FOUND;
					fr->bufferPos->size = 0;
					sem_post(&fr->positionReady);

				}

			} else {
				// for graphics
				sem_wait(&fr->detectionReady);
				fr->buffer->state = EYES_NOT_FOUND;
				sem_post(&fr->detectionReady);	
				fprintf(stderr, " face couldnt fit face.size() %d\n", faces.size());

				// for fusion
				sem_wait(&fr->positionReady);
				fr->bufferPos->state = EYES_NOT_FOUND;
				fr->bufferPos->size = 0;
				sem_post(&fr->positionReady);
			}

		} else {
			prevHCFace = 0;
			// for graphics
			sem_wait(&fr->detectionReady);
			fr->buffer->state = NO_EYES_NO_FACE;
			sem_post(&fr->detectionReady);	

			// for fusion
			sem_wait(&fr->positionReady);
			fr->bufferPos->state = NO_EYES_NO_FACE;
			fr->bufferPos->size = 0;
			sem_post(&fr->positionReady);
		}

		#ifdef CHECK_ET_OVERALL
			clock_gettime(CLOCK_MONOTONIC, &overall_end);
			float oad = (overall_end.tv_sec + overall_end.tv_nsec / 1000000000.0) - (overall_begin.tv_sec + overall_begin.tv_nsec / 1000000000.0);
			printf("oa ET %f ms\n", oad * 1000);		
		#endif

	}
	pthread_exit(NULL);
}

void CAReOCV_ScaleDownImage(FACERECOG_T *fr, int scale) {
	fr->scale = scale;
}

void oCV_loadAttributes(pthread_attr_t *fr) {
	struct sched_param m_param;
	m_param.sched_priority = CAReOCV_PRIORITY;
	pthread_attr_init(fr);
	pthread_attr_setinheritsched(fr, PTHREAD_EXPLICIT_SCHED);
	pthread_attr_setschedpolicy(fr, SCHED_FIFO);
	pthread_attr_setschedparam(fr, &m_param);
}

int CAReOCV_runRecognizer(FACERECOG_T *fr, RECOGBUFFER_T *faces) {
	int rc;

	fr->exit = 1;
	fr->buffer = faces;
	fftr.exit = 1;

	// Face detector
	oCV_loadAttributes(&fr->oCVThreadAttr);
	rc = pthread_create(&fr->oCVThreadID, &fr->oCVThreadAttr, oCV_runRecognizerTask3, (void *) fr);

	if (rc > 0) {
		if (rc == EAGAIN) 
			fprintf(stderr, "[error@CAReOCV] system's limitations\n");
		else if (rc == EINVAL)
			fprintf(stderr, "[error@CAReOCV] invalid attributes\n");
		else if (rc == EPERM)
			fprintf(stderr, "[error@CAReOCV] make sure to run with sudo\n");
		return -1;
	}

	return 0;
}

void CAReOCV_stopRecognizer(FACERECOG_T *fr) {
	fr->exit = 0;
	fftr.exit = 0;
	// release semaphores
	sem_trywait(fr->InFrameReady);
	sem_post(fr->InFrameReady);
	
	sem_trywait(&fr->detectionReady);
	sem_post(&fr->detectionReady);
	
	sem_trywait(&fr->positionReady);
	sem_post(&fr->positionReady);

	// JOIN threads
	pthread_join(fr->oCVThreadID, NULL);

	// destroy attribs
	pthread_attr_destroy(&fr->oCVThreadAttr);
}
