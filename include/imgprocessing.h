#ifndef IMG_PROCESSING_H
#define IMG_PROCESSING_H 

#include "types.h"


#define CAReOCV_PRIORITY 60
#define CAReOCV_LOOKING_CENTER	1
#define CAReOCV_LOOKING_AWAY	2
#define CAReOCV_MAX_EYES		10

typedef struct {
	unsigned char i;
	unsigned int i0;
	unsigned int tl0;
	unsigned int tl1;
	unsigned int c;
	unsigned int i1;
} EYEINDEX_T;

typedef enum {
	EYES_FOUND,			// face and eye detected and open  
	EYES_FOUND_CLOSED,	// the eyes are closed
	EYES_NOT_FOUND,		// face detected but eyes
	NO_EYES_NO_FACE		// no face detected, so no no eyes
} EYESFOUNDSTATE_T;

typedef struct {
	double *data;			// where the x/y positions are allocated 
	size_t size;			// how many eyes were found currently
	EYESFOUNDSTATE_T state;	// eyes were found?
	unsigned char full;
} EYESPOSBUFF_T;

typedef struct {
	std::vector<cv::Rect> rect;
	unsigned char look_dir[CAReOCV_MAX_EYES];
	EYESFOUNDSTATE_T state;	// eyes were found?
	
	// unsigned char *status; 
} RECOGBUFFER_T;

typedef struct {
	pthread_t oCVThreadID;
	pthread_attr_t oCVThreadAttr;
	unsigned exit;
} FACEFITTER_T;

typedef struct {
	cv::CascadeClassifier face_cascade;
	RECOGBUFFER_T *buffer;
	EYESPOSBUFF_T *bufferPos;
	// std::vector<cv::Rect> *faces;
	cv::Mat image;
	int 	height;	// height of the image to process
	int 	width;	// length of the image to process
	int 	scale;	// how much to scale down the picture to process
	sem_t 	*InFrameReady;
	sem_t 	detectionReady;
	sem_t	positionReady;
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