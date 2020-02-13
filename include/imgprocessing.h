#ifndef IMG_PROCESSING_H
#define IMG_PROCESSING_H 

#include "types.h"

#define CAReOCV_PRIORITY 40
#define CAReOCV_LOOKING_LEFT	1
#define CAReOCV_LOOKING_RIGHT	2
#define CAReOCV_LOOKING_CENTER	3
#define CAReOCV_LOOKING_UP		4
#define CAReOCV_LOOKING_DOWN	5
#define CAReOCV_MAX_EYES		10

typedef struct {
	std::vector<cv::Rect> rect;
	unsigned char look_dir[CAReOCV_MAX_EYES];
	unsigned char *status; 
} RECOGBUFFER_T;

typedef struct {
	cv::CascadeClassifier face_cascade;
	cv::CascadeClassifier eyes_cascade;
	RECOGBUFFER_T *buffer;
	// std::vector<cv::Rect> *faces;
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
int CAReOCV_runRecognizer(FACERECOG_T *fr, RECOGBUFFER_T *faces);
void CAReOCV_stopRecognizer(FACERECOG_T *fr);

#endif