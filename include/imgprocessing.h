#ifndef IMG_PROCESSING_H
#define IMG_PROCESSING_H 

#include "types.h"

#define CAReOCV_PRIORITY 40

typedef struct {
	cv::CascadeClassifier face_cascade;
	cv::CascadeClassifier eyes_cascade;
	std::vector<cv::Rect> *faces;
	cv::Mat image;
	int height;	// height of the image to process
	int width;	// length of the image to process
	int scale;	// how much to scale down the picture to process
	sem_t *InFrameReady;
	sem_t detectionReady;
	pthread_t oCVThreadID;
	pthread_attr_t oCVThreadAttr;
	unsigned exit;
} FACERECOG_T;

int initCAReOCV(int height, int width, FACERECOG_T *fr);
void CAReOCV_plugImage2Recognizer(FACERECOG_T *fr, unsigned char **image);
void CAReOCV_ScaleDownImage(FACERECOG_T *fr, int scale);
int CAReOCV_runRecognizer(FACERECOG_T *fr, std::vector<cv::Rect> *faces);
void CAReOCV_stopRecognizer(FACERECOG_T *fr);

#endif