#ifndef CARE_GRAPHICS_H
#define CARE_GRAPHICS_H

#include "types.h"
#include "imgprocessing.h"
#include "vgfont.h" // for display text on screen

#include <opencv2/core/types_c.h>

#define GRAPHICS_PRIORITY	40
#define GRAPHICS_RGBA32_RED 	GRAPHICS_RGBA32(0x00, 0x00, 0xff, 0xff)
#define GRAPHICS_RGBA32_YELLOW 	GRAPHICS_RGBA32(0x00, 0xff, 0xff, 0xff)

extern	GRAPHICS_RESOURCE_HANDLE img_overlay;
// extern 	GRAPHICS_RESOURCE_HANDLE img_overlay2;

typedef struct {
	// std::vector<cv::Rect> 	sharedFaces;
	RECOGBUFFER_T			sharedFaces;
	sem_t 					*semSharedFaces;
	double 					heartRate;
	pthread_t graphicsTID;
	pthread_attr_t graphicsTAttr;
	unsigned exit;
} GRAPHCARE_T;


void initGraphics(PORTUSERDATA_T *usr, GRAPHCARE_T *gr);
void getDisplaySizeAndScale2OpenCV(PORTUSERDATA_T *usr);
void drawSquareOnFace(GRAPHICS_RESOURCE_HANDLE *img, CvRect *r);
int runGraphics(GRAPHCARE_T *gr);
void stopGraphics(GRAPHCARE_T *gr);

void *graphicTask(void *arg);

#endif