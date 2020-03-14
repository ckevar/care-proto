#include "heartrate.h"
#include "fft.h"
#include "timeutils.h"

#include <errno.h>
#include <stdio.h>
#include <cstring>
#include <cstdlib>

#ifdef SAVE_HR_SPECTRUM
	FILE *fdhrs;
#endif

void searchMax(complex *y, HeartRate_t *hr, unsigned *dummy) {
	*dummy = 0;
	hr->peak = 0;
	double bin;
	#ifdef SAVE_HR_SPECTRUM
		fprintf(fdhrs, "%f ", y[0].re * y[0].re + y[0].im * y[0].im);
	#endif
	for (unsigned k = hr->FLi; k < hr->FHi; k++) {
		bin = y[k].re * y[k].re + y[k].im * y[k].im;
		
		#ifdef SAVE_HR_SPECTRUM
			fprintf(fdhrs, "%f ", bin);
		#endif 

		if (bin > hr->peak) {
			hr->peak = bin;
			*dummy = k;
		}
	}

	#ifdef SAVE_HR_SPECTRUM
		fprintf(fdhrs, "\n");
	#endif
}

void fillwithRandom(complex *x, size_t length) {
	for (size_t i = 0; i < length; ++i) {
		x[i].re = 1;
		// x[i].re =  rand() % 100 - 50;
		// x[i].re =  rand() % (SIGNAL_THRESHOLD + 1) + SIGNAL_THRESHOLD;
		x[i].im = 0.0;
	}
}

void *runMeasureTask(void *arg) {
	HeartRate_t *hr = (HeartRate_t *) arg;
	complex	W[N_HR - 1];		// precomputed twiddle factors	
	complex X[N_HR];			// heart rate samples taken from the sensors
	complex Y[N_HR];
	unsigned redLED, dummy;
	unsigned freqIndex;
	Kalman1Linear kalman;		// Linear Kalman single variable
	struct timespec t;
	struct timespec now;
	unsigned j = 0;
	kalman.setX0(1);
	kalman.setR(2.78);
	kalman.setP0(14);
	*hr->bpm = 0.0;

	//fillwithRandom(X, N_HR);

	time_t tt;
	srand((unsigned)time(&tt));;

	#ifdef LOG_MAX30102_FREQOUT
		FILE *fphr;
		fphr = fopen("freqMAX30102.dat", "w");
	#endif

	#ifdef SAVE_HR_SPECTRUM
		fdhrs = fopen("HR_spectrum.dat", "w");
	#endif

	// precomputes the twiddle factor for FFT
	createTwiddleTable(W, EXP_HR);
	while(hr->exit) {
		hr->dev.checkNewFifoData();
		clock_gettime(CLOCK_MONOTONIC, &t);
		time_add_ms(&t, RELATIVE_ACTIVATION_TIME);

		hr->dev.readFifo(&redLED, &dummy);

		if (redLED > SIGNAL_THRESHOLD) {		// finger on sesor

			X[j % N_HR].re = redLED;
			X[j % N_HR].im = 0.0;
			j++;

			memcpy(Y, X, sizeof(complex) * N_HR);
			bit_rev(Y, EXP_HR);
			dft(Y, EXP_HR, W);

			searchMax(Y, hr, &freqIndex);

			#ifdef LOG_MAX30102_FREQOUT
				fprintf(fphr, "%d\n", freqIndex);
			#endif

			if (hr->peak < 5000)	{			// heart rate acceptable value
				/* do I need a protection for a single variable? */
				// *hr->bpm = kalman.update(freqIndex * FS_HR * 60.0 / N_HR);
				*hr->bpm = freqIndex * FS_HR * 60.0 / N_HR;
			} else								// no aceptable value
				*hr->bpm = -2;	
		} else 									// no finger on sensor
			*hr->bpm = -1;

		sem_post(&hr->hrReady);

		clock_gettime(CLOCK_MONOTONIC, &now);
		if(timecmp(now, t) > 0) fprintf(stderr, "[warning] deadline miss at sensor\n");
		clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &t, NULL);
	}
	hr->dev.shutdown();
	hr->dev.terminate();
	pthread_exit(NULL);
}


int HeartRate_init(HeartRate_t *hr) {
	hr->FLi = (unsigned) (LOW_HR * N_HR / FS_HR);
	hr->FHi = (unsigned) (HIGH_HR * N_HR / FS_HR) + 1;
	hr->exit = 1;
	// inits i2c port
	if (hr->dev.init() < 0) {
		fprintf(stderr, "[error] init i2c port\n");
		return -1;
	}
	// configures the MAX30102
	if (hr->dev.settings() < 0) {
		fprintf(stderr, "[error] settings\n");
		return -1;
	}
	sem_init(&hr->hrReady, 0, 0);
	return 0;
}

void HR_loadAttributes(HeartRate_t *hr) {	
	struct sched_param m_param;
	m_param.sched_priority = HR_PRIORITY;
	pthread_attr_init(&hr->HRThreadAttr);
	pthread_attr_setinheritsched(&hr->HRThreadAttr, PTHREAD_EXPLICIT_SCHED);
	pthread_attr_setschedpolicy(&hr->HRThreadAttr, SCHED_FIFO);
	pthread_attr_setschedparam(&hr->HRThreadAttr, &m_param);
}

int HeartRate_runMeasure(HeartRate_t *hr, double *bpm) {
	int rc;

	hr->bpm = bpm;
	HR_loadAttributes(hr);	
	rc = pthread_create(&hr->HRThreadID, &hr->HRThreadAttr, runMeasureTask, (void *) hr); 

	if (rc == 0) 
		return 0;
	else if (rc == EAGAIN) 
		fprintf(stderr, "[error@HR] system's limitations\n");
	else if (rc == EINVAL)
		fprintf(stderr, "[error@HR] invalid attributes\n");
	else if (rc == EPERM)
		fprintf(stderr, "[error@HR] make sure to run with sudo\n");
		
	hr->dev.terminate();
	return -1;
}

void HeartRate_stopMeasure(HeartRate_t *hr) {
	hr->exit = 0;
	pthread_join(hr->HRThreadID, NULL);
	pthread_attr_destroy(&hr->HRThreadAttr);
}