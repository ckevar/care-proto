#ifndef TYPES_H
#define TYPES_H 

#include "interface/mmal/mmal.h"
#include "interface/mmal/util/mmal_connection.h"
#include <opencv2/objdetect/objdetect.hpp>

typedef struct {
	unsigned video_width;
	unsigned video_height;
	int preview_width;
	int preview_height;
	int opencv_width;
	int opencv_height;
	float video_fps;
	MMAL_POOL_T *camVideoPortPool;
	unsigned char *image;
	sem_t outFrameReady;
} PORTUSERDATA_T;


#endif