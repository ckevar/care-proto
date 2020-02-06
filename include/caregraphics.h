#ifndef CARE_GRAPHICS_H
#define CARE_GRAPHICS_H

#include "types.h"
#include "vgfont.h" // for display text on screen

#include <opencv2/core/types_c.h>

#define GRAPHICS_PRIORITY	40

extern	GRAPHICS_RESOURCE_HANDLE img_overlay;
// extern 	GRAPHICS_RESOURCE_HANDLE img_overlay2;

void initGraphics(PORTUSERDATA_T *usr, GRAPHCARE_T *gr);
void getDisplaySizeAndScale2OpenCV(PORTUSERDATA_T *usr);
void drawSquareOnFace(GRAPHICS_RESOURCE_HANDLE *img, CvRect *r);
int runGraphics(GRAPHCARE_T *gr);
void stopGraphics(GRAPHCARE_T *gr);

void *graphicTask(void *arg);

#endif