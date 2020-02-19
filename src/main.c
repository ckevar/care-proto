#include <stdio.h>
#include <stdlib.h>
#include <signal.h>

#include "types.h"
#include "campi.h"
#include "imgprocessing.h"
#include "caregraphics.h"
#include "heartrate.h"
#include "definitions.h"

#include "bcm_host.h"
#include "interface/vcos/vcos.h"
#include "interface/vcos/vcos.h"

#include "interface/mmal/mmal.h"
#include "interface/mmal/util/mmal_default_components.h"
#include "interface/mmal/util/mmal_connection.h"
#include "interface/mmal/util/mmal_util.h"
#include "interface/mmal/util/mmal_util_params.h"

#include "vgfont.h" // for display text on screen

sem_t BLOCK_MAIN;

void INT_HANDLER(int sig) {
	sem_post(&BLOCK_MAIN);
}

void videoRes(PORTUSERDATA_T *usr) {
	// Image resolution to be displayed
	usr->preview_width = CARE_RENDER_IMG_WIDTH;
	usr->preview_height = CARE_RENDER_IMG_HEIGHT;
	// Image resolution to be acquired for processing
	usr->video_width = CARE_VIDEO_IMG_WIDTH;
	usr->video_height = CARE_VIDEO_IMG_HEIGHT;
	// Image resolution to process
	usr->opencv_width = CARE_VIDEO_IMG_WIDTH / CARE_SCALEDOWN_IMG;
	usr->opencv_height = CARE_VIDEO_IMG_HEIGHT / CARE_SCALEDOWN_IMG;
}

int main(int argc, char** argv) {
	PORTUSERDATA_T 	userdata;		// general userdata for mmal
	FACERECOG_T 	faceRecog;		// Face Recognizer
	GRAPHCARE_T 	graphRes;		// shared graphical resources
	HeartRate_t hr;					// HearRate sensor
	FUSION_T 	fusion;				// fusion type

	printf("Running...\n");
	CamPi cam(&userdata);

	bcm_host_init();  				// init the bcm
	signal(SIGINT, INT_HANDLER); 	// to unblock the main thread 
	videoRes(&userdata);			// sets the video resolution

	getDisplaySizeAndScale2OpenCV(&userdata);
	initGraphics(&userdata, &graphRes);

	// init opencv
	if (initCAReOCV(CARE_VIDEO_IMG_HEIGHT, CARE_VIDEO_IMG_WIDTH, &faceRecog) < 0) 
		fprintf(stderr,"[error] could not initialize OpenCV\n");

	CAReOCV_ScaleDownImage(&faceRecog, CARE_SCALEDOWN_IMG);
	CAReOCV_plugImage2Recognizer(&faceRecog, &userdata.image);


	// init camera component
	if (cam.initCam() < 0) return -1;

	// init render component
	if (cam.initRender() < 0) return -1;

	// connects render to camera
	if (cam.plugRender2Cam() < 0) return -1;

	cam.loadBuffers();

	// init heart rate sensor	
	if (HeartRate_init(&hr) < 0) 
		fprintf(stderr, "[error] could not init hr sensor\n");

	// init fusion
	initFusion(&fusion);


	// init semaphores
	sem_init(&userdata.outFrameReady, 0, 0);
	sem_init(&faceRecog.detectionReady, 0, 1);
	sem_init(&BLOCK_MAIN, 0, 0);

	// passing semaphore from heart rate module to fusion	
	fusion.hrReady = &hr.hrReady;

	// passing semaphores from MMAL to opencv
	faceRecog.InFrameReady = &userdata.outFrameReady;

	// pasing semaphores from opencv to graphics
	graphRes.semSharedFaces = &faceRecog.detectionReady;

	// connect fusion with graphics 
	graphRes.fusionBuffer = &fusion.buffer; 

	// THREAD
	if(HeartRate_runMeasure(&hr, &fusion.bpm) < 0) return -1;					// heart-rate thread
	if(CAReOCV_runRecognizer(&faceRecog, &graphRes.sharedFaces) < 0) return -1;	// recognizer thread
	if(runFusion(&fusion) < 0) return -1;										// fusion thread 
	if(runGraphics(&graphRes) < 0) return -1;									// graphics thread

	// block the main thread (this thread)
	sem_wait(&BLOCK_MAIN);	
	fprintf(stderr, "\nstopping threads ... \n");

	// close all threads
	cam.coronavirus();
	fprintf(stderr, "Cam Module STOPPED\n");
	
	stopGraphics(&graphRes);
	fprintf(stderr, "Graphics STOPPED\n");
	
	CAReOCV_stopRecognizer(&faceRecog);
	fprintf(stderr, "Recognizer STOPPED \n");
	
	HeartRate_stopMeasure(&hr);
	fprintf(stderr, "Heart Rate  STOPPED  \n");
	
	fusion_stop(&fusion);
	fprintf(stderr, "Fusion module STOPPED  \n");

	return 0;
}

