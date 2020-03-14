#include "caregraphics.h"
#include "timeutils.h"

#include "bcm_host.h"
#include <errno.h>


GRAPHICS_RESOURCE_HANDLE img_overlay;

uint32_t DISPLAYSIZE_WH[2]; 	// display resolution
float 	SCALE_WH[2];			// scale from video resolution to display resolution

void getDisplaySizeAndScale2OpenCV(PORTUSERDATA_T *usr) {
	// gets the display size and the scales from video resolution to display resolution
	graphics_get_display_size(0, DISPLAYSIZE_WH, DISPLAYSIZE_WH + 1);
	
	//Rescale parameter to draw the markers where are they displayed from image processed to image displayed
	*SCALE_WH = (float) *(DISPLAYSIZE_WH) / (float) usr->opencv_width;
	*(SCALE_WH + 1) = (float) *(DISPLAYSIZE_WH + 1) / (float) usr->opencv_height;
}

void initGraphics(PORTUSERDATA_T *usr, GRAPHCARE_T *gr) {
	// init graphics
	gx_graphics_init("/opt/vc/src/hello_pi/hello_font");


	gx_create_window(0, usr->opencv_width, usr->opencv_height, GRAPHICS_RESOURCE_RGBA32, &img_overlay);
	graphics_resource_fill(img_overlay, 0, 0, GRAPHICS_RESOURCE_WIDTH, GRAPHICS_RESOURCE_HEIGHT, GRAPHICS_RGBA32(0xff, 0, 0, 0x55));

	graphics_display_resource(img_overlay, 0, 1, 0, 0, *DISPLAYSIZE_WH, *(DISPLAYSIZE_WH + 1), VC_DISPMAN_ROT0, 1);
	gr->exit = 1;
}

void drawSquareOnFace(GRAPHICS_RESOURCE_HANDLE *img, CvRect *r) {
	graphics_resource_fill(*img, r->x, r->y, r->width, r->height, GRAPHICS_RGBA32(0, 0, 0xff, 0x88));
}

void bpm2Text(double bpm, char *text, uint32_t *fontHRColor) {
	if (bpm == -1) {
		sprintf(text, "HR: NO FINGER");
		*fontHRColor = GRAPHICS_RGBA32_RED;
	} else if (bpm == -2) {
		sprintf(text, "HR: WARMING UP");
		*fontHRColor = GRAPHICS_RGBA32_YELLOW;

	} else if ((bpm > 35) && (bpm < 220)) {
		sprintf(text, "HR: %.0f ", bpm);
		*fontHRColor = GRAPHICS_RGBA32_GREEN;

	} else if ((bpm >= 0) && (bpm < 35)) {
		sprintf(text, "HR: U DEAD?");
		*fontHRColor = GRAPHICS_RGBA32_RED;
	} else {
		sprintf(text, "HR: SENSOR ISSUES");
		*fontHRColor = GRAPHICS_RGBA32_RED;
	}
}

int driverState2Text(DRIVER_STATE_T ds, char *msg, uint32_t *fontColor, unsigned *wpos) {
	if (ds == DRIVER_WARNING) {
		wpos[0] = 0;
		wpos[1] = 120;
		sprintf(msg, "WARNING");
		*fontColor = GRAPHICS_RGBA32_YELLOW;
		return 1;
	} else if (ds == DRIVER_STOP) {
		wpos[0] = 120;
		wpos[1] = 80;
		sprintf(msg, "    STOP\nrequested");
		*fontColor = GRAPHICS_RGBA32_RED;
		return 1;
	}
	return 0;
}
// requ ested

int gazeState2Text(GAZE_STATE_T ds, char *msg, uint32_t *fontColor) {
	if (ds == EYES_FOCUS) {
		sprintf(msg, "Focus");
		*fontColor = GRAPHICS_RGBA32_GREEN;
		return 1;
	} else if (ds == EYES_AWAY_LEFT) {
		sprintf(msg, "Gaze at left");
		*fontColor = GRAPHICS_RGBA32_ORANGE;
		return 1;
	}
	else if (ds == EYES_AWAY_RIGHT) {
		sprintf(msg, "Gaze at right");
		*fontColor = GRAPHICS_RGBA32_ORANGE;
		return 1;
	}
	else if (ds == EYES_CLOSED) {
		sprintf(msg, "Eyes closed");
		*fontColor = GRAPHICS_RGBA32_RED;
		return 1;
	}
	else if (ds == FACE_ERROR) {
		sprintf(msg, "Place yourself in the camera range");
		*fontColor = GRAPHICS_RGBA32_RED;
		return 1;
	}
	return 0;
}

void *graphicTask(void *arg) {
	GRAPHCARE_T *gr = (GRAPHCARE_T *) arg;
	struct timespec t;
	unsigned i, fontSize;
	char msgHR[25], msgDriverState[25], msgGazeState[25];
	uint32_t fontHRColor, fontColorDriverState, fontColorGazeState;
	uint32_t rcDriver, rcGaze;
	unsigned warnPosition[2];
	double bpm;

	DRIVER_STATE_T driverState;
	GAZE_STATE_T	gazeState;
	EYESFOUNDSTATE_T eyeState;

	std::vector<cv::Rect> localFaces;
	fontSize = 15;

	clock_gettime(CLOCK_MONOTONIC, &t);
	time_add_ms(&t, 33);

	while(gr->exit) {
		graphics_resource_fill(img_overlay, 0, 0, GRAPHICS_RESOURCE_WIDTH, GRAPHICS_RESOURCE_HEIGHT, GRAPHICS_RGBA32(0, 0, 0, 0x00));
		
		if(sem_trywait(gr->semSharedFaces) == 0) {
			localFaces = gr->sharedFaces.rect;
			eyeState = gr->sharedFaces.state; 
			sem_post(gr->semSharedFaces);
			if (eyeState == EYES_FOUND) {
				for (i = 0; i < localFaces.size(); i++) {
					drawSquareOnFace(&img_overlay, (CvRect *)&localFaces[i]);
				}
			}
		}

		bpm = *gr->fusionBuffer->bpm;
		driverState = gr->fusionBuffer->driverState;
		gazeState = gr->fusionBuffer->gazeState;

		bpm2Text(bpm, msgHR, &fontHRColor);
		rcDriver = driverState2Text(driverState, msgDriverState, &fontColorDriverState, warnPosition);
		rcGaze = gazeState2Text(gazeState, msgGazeState, &fontColorGazeState);

    	graphics_resource_render_text_ext(img_overlay, 0, 160,
			GRAPHICS_RESOURCE_WIDTH,
			GRAPHICS_RESOURCE_HEIGHT,
			fontHRColor, 						// foreground color
	  		GRAPHICS_RGBA32(0, 0, 0, 0x00), 	// background color
			msgHR, strlen(msgHR), fontSize);

    	if (rcDriver) 
    		graphics_resource_render_text_ext(img_overlay, warnPosition[0], warnPosition[1],
    			GRAPHICS_RESOURCE_WIDTH,
    			GRAPHICS_RESOURCE_HEIGHT,
    			fontColorDriverState, 			// foregounrd color
    			GRAPHICS_RGBA32(0, 0, 0, 0x00), // background color
    			msgDriverState, strlen(msgDriverState), fontSize);
    	
    	if(rcGaze)
    		graphics_resource_render_text_ext(img_overlay, 0, 140,
    			GRAPHICS_RESOURCE_WIDTH,
    			GRAPHICS_RESOURCE_HEIGHT,
    			fontColorGazeState, 			// foregounrd color
    			GRAPHICS_RGBA32(0, 0, 0, 0x00), // background color
    			msgGazeState, strlen(msgGazeState), fontSize);
		
    	graphics_update_displayed_resource(img_overlay, 0, 0, 0, 0);
		clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &t, NULL);
		time_add_ms(&t, 33);
	}
	graphics_display_resource(img_overlay, 0, 1, 0, 0, DISPLAYSIZE_WH[0], DISPLAYSIZE_WH[1], VC_DISPMAN_ROT0, 0);
	graphics_delete_resource(img_overlay);
	pthread_exit(NULL);
}

void graphics_loadAttributes(GRAPHCARE_T *gr) {
	struct sched_param m_param;
	m_param.sched_priority = GRAPHICS_PRIORITY;

	pthread_attr_init(&gr->graphicsTAttr);
	pthread_attr_setinheritsched(&gr->graphicsTAttr, PTHREAD_EXPLICIT_SCHED);
	pthread_attr_setschedpolicy(&gr->graphicsTAttr, SCHED_FIFO);
	pthread_attr_setschedparam(&gr->graphicsTAttr, &m_param);
}

int runGraphics(GRAPHCARE_T *gr) {
	int rc;
	graphics_loadAttributes(gr);
	rc = pthread_create(&gr->graphicsTID, &gr->graphicsTAttr, graphicTask, (void *) gr);

	if (rc == 0) 
		return 0;
	else if (rc == EAGAIN) 
		fprintf(stderr, "[error@graphics] system's limitations\n");
	else if (rc == EINVAL)
		fprintf(stderr, "[error@graphics] invalid attributes\n");
	else if (rc == EPERM)
		fprintf(stderr, "[error@graphics] make sure to run with sudo\n");

	graphics_delete_resource(img_overlay);
	return -1;
}


void stopGraphics(GRAPHCARE_T *gr) {
	gr->exit = 0;
	pthread_join(gr->graphicsTID, NULL);
	pthread_attr_destroy(&gr->graphicsTAttr);
}