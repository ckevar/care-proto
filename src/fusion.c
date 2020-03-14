#include "fusion.h"
#include "MLED7219.h"

#include <cstdio>
#include <cstdlib>
#include <errno.h>
#include <time.h>

int initFusion(FUSION_T *fs) {
	fs->exit = 1;
	fs->buffer.bpm = &fs->bpm;		// temporal assignation
	fs->eyes.data = fs->eyespos;	// pointer to the eyes position stored in the fusion part
	fs->eyes.size = 0;
	return 1;
}

inline double subHPFilter(double bpm) {
	static double k = 0.9;		// coef to be determined
	static double stableBPM = 60;
	stableBPM = k * stableBPM + (1 - k) * bpm;
	// Y(k) = A * Y(k-1) + (1 - A) * x(k)
	return stableBPM;
}

double varianceEstimator(double sbpm, double bpm) {
	static double a[] = {-0.01666, -0.06667, -0.05, -0.03333, 0, 0.0166667, 0.03333, 0.05, 0.06667}; //;
	static double x[] = {60, 60, 60, 60, 60, 60, 60, 60, 60, 60};
	double aux = 0.0;
	
	x[8] = bpm;

	for (int i = 0; i < 9; ++i) {
		aux += x[i] * a[i];
		x[i] = x[i + 1];
	}

	if (aux < 0) aux *= -1.0;

	return aux / sbpm * 100.0;
}

int MocosTable(GAZE_STATE_T eyesStatus, double bpm, double vbpm, FUSIONBUFFER_T *buff, MLED7219 *ledm) {

	buff->gazeState = eyesStatus;

	if ((eyesStatus == EYES_AWAY_LEFT) || (eyesStatus == EYES_AWAY_RIGHT)) {
		if (bpm < HR_LOW) {
			buff->driverState = DRIVER_STOP;
			ledm->displayStopSymbol();
			ledm->intensityBreath();
			return 1;
		} else if ((bpm >= HR_LOW) && (bpm <= HR_HIGH)) {
			if (vbpm > HRV_OVER30) {
				buff->driverState = DRIVER_STOP;
				ledm->displayHeart();
				ledm->intensityBreath();
				return 1;
			} else if (vbpm < HRV_BELOW25) {
				buff->driverState = DRIVER_STOP;
				ledm->displayStopSymbol();
				ledm->intensityBreath();
				return 1;
			} else {
				buff->driverState = DRIVER_WARNING;
				ledm->displayQuestionMark();
				ledm->intensityBreath();
				return 1;
			}
		}
	}

	if (eyesStatus == EYES_FOCUS) {
		if (bpm < HR_LOW) {
			buff->driverState = DRIVER_STOP;
			ledm->displayHeart();
			ledm->intensityBreath();
			return 1;
		} else if (bpm > HR_HIGH) {
			if(vbpm < HRV_BELOW25) {
				buff->driverState = DRIVER_WARNING;
				ledm->displayQuestionMark();
				ledm->intensityBreath();
				return 1;
			} else {
				buff->driverState = DRIVER_STOP;
				ledm->displayStopSymbol();
				ledm->intensityBreath();
				return 1;
			}
		} else  {
			if (vbpm < HRV_BELOW25) {
				buff->driverState = DRIVER_NORMAL;
				ledm->smile();
				ledm->setIntensity(12);
				return 0;
			} else if (vbpm > HRV_OVER30) {
				buff->driverState = DRIVER_STOP;
				ledm->displayStopSymbol();
				ledm->intensityBreath();
				return 1;
			} else {
				buff->driverState = DRIVER_WARNING;
				ledm->displayQuestionMark();
				ledm->intensityBreath();
				return 1;
			}
		}
	}

	if (eyesStatus == EYES_CLOSED) {
		if (bpm < HR_LOW) {
			buff->driverState = DRIVER_STOP;
			ledm->displayStopSymbol();
			ledm->intensityBreath();
			return 1;
		} else if (bpm > HR_HIGH) {
			buff->driverState = DRIVER_STOP;
			ledm->displayHeart();
			ledm->intensityBreath();
			return 1;
		} else {
			if (vbpm < HRV_BELOW25) {
				buff->driverState = DRIVER_WARNING;
				ledm->displayQuestionMark();
				ledm->intensityBreath();
				return 1;
			} else {
				buff->driverState = DRIVER_STOP;
				ledm->displayHeart();
				ledm->intensityBreath();
				return 1;
			}
		}
	}

	return -1;

}

GAZE_STATE_T focusEstimator (EYESPOSBUFF_T *eyes) {
	static unsigned framesEyesFoundClosed = 0;
	static unsigned framesFaceNotFound = 0;
	static unsigned framesGazeAway = 0;

	if (eyes->state == EYES_FOUND_CLOSED) {
		framesEyesFoundClosed++;
		if (framesEyesFoundClosed > EYES_CLOSED_FRAME_THRESHOLD) {	// this threshold can be smalled depending on the speed
			return EYES_CLOSED;
		}
	}

	if (eyes->state == NO_EYES_NO_FACE) {
		framesFaceNotFound ++;
		if (framesFaceNotFound > FACE_NOT_FOUND_THRESHOLD) {
			return FACE_ERROR;
		}
	}

	framesEyesFoundClosed = 0;
	framesFaceNotFound = 0;

	if (eyes->data[0] > 0.59) {
		framesGazeAway++;
		if (framesGazeAway > 3) {
			return EYES_AWAY_LEFT;
		}
		else
			return EYES_FOCUS;

	} else if (eyes->data[0] < 0.47) {
		framesGazeAway++;
		if(framesGazeAway > 3) {
			return EYES_AWAY_RIGHT;
		}
		else 
			return EYES_FOCUS;

	} else {
		framesGazeAway = 0;
		return EYES_FOCUS;
	}
	return FACE_ERROR; 
}

void *fusionTask(void *arg) {
	FUSION_T *fs = (FUSION_T *) arg;
	double bpm, sbpm, vbpm;
	MLED7219 ledmatrix;

	fs->buffer.bpm = &bpm; 
	GAZE_STATE_T eyesStatus = EYES_FOCUS;

	if(ledmatrix.init(0) < 0)
		fprintf(stderr, "[error] could init LED matrix\n");

	while(fs->exit) {

		/** BPM ANALYZER **/
		sem_wait(fs->hrReady);
		bpm = fs->bpm;
		// sem_post(&fs->heartRate)
		sbpm = subHPFilter(bpm);
		vbpm = varianceEstimator(sbpm, bpm);
		/** BPM ANALYZER END **/

		/** VIEW POINT ANALYZER **/
		if(sem_trywait(fs->eyePosReady) == 0) {
			eyesStatus = focusEstimator(&fs->eyes);
			sem_post(fs->eyePosReady);
		}
		/** VIEW POINT END **/

		/** LOGICAL MERGER **/
		MocosTable(eyesStatus, bpm, vbpm, &fs->buffer, &ledmatrix);
		/** END LOGICAL MERGER **/
	}
	ledmatrix.clear();
	ledmatrix.destroy();
	pthread_exit(NULL);
}

void fusion_loadAttributes(FUSION_T *gr) {
	struct sched_param m_param;
	m_param.sched_priority = FUSION_PRIORITY;

	pthread_attr_init(&gr->fusionTAttr);
	pthread_attr_setinheritsched(&gr->fusionTAttr, PTHREAD_EXPLICIT_SCHED);
	pthread_attr_setschedpolicy(&gr->fusionTAttr, SCHED_FIFO);
	pthread_attr_setschedparam(&gr->fusionTAttr, &m_param);
}


int runFusion(FUSION_T *fs) {
	int rc;
	fusion_loadAttributes(fs);
	rc = pthread_create(&fs->fusionTID, &fs->fusionTAttr, fusionTask, (void *) fs);

	if (rc == 0) 
		return 0;
	else if (rc == EAGAIN) 
		fprintf(stderr, "[error@graphics] system's limitations\n");
	else if (rc == EINVAL)
		fprintf(stderr, "[error@graphics] invalid attributes\n");
	else if (rc == EPERM)
		fprintf(stderr, "[error@graphics] make sure to run with sudo\n");

	return -1;
}

void fusion_stop(FUSION_T *fs) {
	fs->exit = 0;
	sem_post(fs->hrReady);
	pthread_join(fs->fusionTID, NULL);
	pthread_attr_destroy(&fs->fusionTAttr);
}