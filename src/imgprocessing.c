#include "imgprocessing.h"

// #include <opencv2/core/core_c.h>
#include <opencv2/imgproc/imgproc.hpp>
#include <pthread.h>


int initCAReOCV(int height, int width, FACERECOG_T *fr) {
	/********* setup opencv */
	if(!fr->face_cascade.load("/installation/OpenCV-/share/opencv4/haarcascades/haarcascade_frontalface_alt.xml")) 
		return -1;

	if(!fr->eyes_cascade.load("/installation/OpenCV-/share/opencv4/haarcascades/haarcascade_eye_tree_eyeglasses.xml"))
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


void *oCV_runRecognizerTask(void *arg) {
	FACERECOG_T *fr = (FACERECOG_T *) arg;
	struct timespec t1;
	cv::Mat scaledImg, faceROI;
	std::vector<cv::Rect> faces;
	std::vector<cv::Rect> eyes;
	FILE *fp;
	fp = fopen("face.txt", "a");
	scaledImg.create(fr->height / fr->scale, fr->width / fr->scale, CV_8UC1);

	clock_gettime(CLOCK_MONOTONIC, &t1);

	while (fr->exit) {
		sem_wait(fr->InFrameReady);
		// oCV_checkFrameRate(&t1);
		cv::resize(fr->image, scaledImg, scaledImg.size(), 0, 0, cv::INTER_LINEAR);
		fr->face_cascade.detectMultiScale(scaledImg, faces, 1.4, 3, 0, cv::Size(100, 100), cv::Size(150, 150));
		
		/*EYE DETECTOR */
		eyes.clear();
		for (unsigned i = 0; i < faces.size(); i++) {
			faceROI = scaledImg(faces[i]);
			fr->eyes_cascade.detectMultiScale(faceROI, eyes, 1.4, 3, 0, cv::Size(20, 20)); 
			for (unsigned j = 0; j < eyes.size(); ++j) {
				eyes[j].x += faces[i].x; 
				eyes[j].y += faces[i].y; 
			}
		}
		// THIS IS NOT A GOOD WAY TO REMOVE EXTRA EYES FOUND
		while (eyes.size() > 2*faces.size()) {
			eyes.pop_back();
		}
		/*END EYE DETECTOR*/
		fprintf(fp, "%d %d\n", faces.size(), eyes.size());
		sem_wait(&fr->detectionReady);
		*fr->faces = eyes;
		sem_post(&fr->detectionReady);
	}

	pthread_exit(NULL);
}

void CAReOCV_ScaleDownImage(FACERECOG_T *fr, int scale) {
	fr->scale = scale;
}

void oCV_loadAttributes(FACERECOG_T *fr) {
	struct sched_param m_param;
	m_param.sched_priority = CAReOCV_PRIORITY;
	pthread_attr_init(&fr->oCVThreadAttr);
	pthread_attr_setinheritsched(&fr->oCVThreadAttr, PTHREAD_EXPLICIT_SCHED);
	pthread_attr_setschedpolicy(&fr->oCVThreadAttr, SCHED_FIFO);
	pthread_attr_setschedparam(&fr->oCVThreadAttr, &m_param);
}

int CAReOCV_runRecognizer(FACERECOG_T *fr, std::vector<cv::Rect> *faces) {
	int rc;

	fr->exit = 1;
	fr->faces = faces;
	oCV_loadAttributes(fr);

	rc = pthread_create(&fr->oCVThreadID, &fr->oCVThreadAttr, oCV_runRecognizerTask, (void *) fr);

	if (rc == 0) 
		return 0;
	else if (rc == EAGAIN) 
		fprintf(stderr, "[error@CAReOCV] system's limitations\n");
	else if (rc == EINVAL)
		fprintf(stderr, "[error@CAReOCV] invalid attributes\n");
	else if (rc == EPERM)
		fprintf(stderr, "[error@CAReOCV] make sure to run with sudo\n");

	return -1;
}

void CAReOCV_stopRecognizer(FACERECOG_T *fr) {
	fr->exit = 0;

	// release semaphores
	sem_trywait(fr->InFrameReady);
	sem_post(fr->InFrameReady);
	sem_trywait(&fr->detectionReady);
	sem_post(&fr->detectionReady);

	pthread_join(fr->oCVThreadID, NULL);
	pthread_attr_destroy(&fr->oCVThreadAttr);
}