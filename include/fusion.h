#ifndef FUSION_H
#define FUSION_H

#include "imgprocessing.h"

#include <pthread.h>
#include <semaphore.h>

#define FUSION_PRIORITY 50
#define EYES_CLOSED_FRAME_THRESHOLD 3
#define EYES_AWAY_FRAME_THRESHOLD 3
#define FACE_NOT_FOUND_THRESHOLD 3

typedef enum {
	EYES_AWAY,		// eyes from the road
	EYES_FOCUS,		// eyes on the road
	EYES_CLOSED,	// face detect but eyes
	FACE_ERROR
} EYES_STATE_T;

typedef enum {
	HR_LOW = 55,	// below 55 bpm
	HR_NORMAL,		// between 55 and 95
	HR_HIGH = 95	// over 95
} HR_STATE_T;

typedef enum {
	HRV_BELOW25 = 25,	// below 25% over normal bpm
	HRV_BTWN25_30,	// between 25% and 30% over the normal bpm
	HRV_OVER30 = 30		// over 30% of bpm
} HRV_STATE_T;

typedef enum {
	DRIVER_NORMAL,
	DRIVER_WARNING,
	DRIVER_STOP

} DRIVER_STATE_T;

typedef struct {
	DRIVER_STATE_T driverState;
	double *bpm;
} FUSIONBUFFER_T;

typedef struct {
	FUSIONBUFFER_T buffer;
	unsigned exit;
	double bpm;
	sem_t *hrReady;
	sem_t *eyePosReady;
	EYESPOSBUFF_T eyes;
	double eyespos[4];

	pthread_t fusionTID;
	pthread_attr_t fusionTAttr;

} FUSION_T;

int initFusion(FUSION_T *fs);
int runFusion(FUSION_T *fs);
void fusion_stop(FUSION_T *fs);

#endif