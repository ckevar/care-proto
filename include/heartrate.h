#ifndef HEART_RATE_H
#define HEART_RATE_H 

#include "max30102.h"
#include "fcomplex.h"
#include <pthread.h>
#include <semaphore.h>

#define HR_PRIORITY	80 		// priority task
#define LOW_HR	0.6666 	// Lowest rate in Hz = 40 bpm / 60s
#define HIGH_HR 3.3333 	// Highest rate in Hz = 200 bpm / 60s



#ifdef MAX30102_100HZ
	#define N_HR 		2048 	// frequency points in the FFT
	#define EXP_HR 		11		// = log2(N)
	#define FS_HR 		100	// Sampling frequency of the sensor
	#define RELATIVE_ACTIVATION_TIME 9 // it should be a period of 10 ms, becasue
									// the sampling frequency is 100Hz. However,
									// based in the fact that the sensor wont be read
									// at exactly 10 ms, it's better to activate the
									// the funtion 1 ms ealier and do a busy-wait until
									// has the data ready. From experiments, the data 
									// is ready from 9.41 to 10.51 ms.

#else
	#define N_HR 		1024 	// frequency points in the FFT
	#define EXP_HR 		10 		// = log2(N)	
	#define FS_HR 		50
	#define RELATIVE_ACTIVATION_TIME 19

#endif

#define SIGNAL_THRESHOLD 140000	// Experimental based, it might change based on the sking color
								// over this value a finger is connected.

typedef struct {
	MAX30102 dev;

	double *bpm;
	sem_t	hrReady;

	double peak;
	pthread_t HRThreadID;
	pthread_attr_t HRThreadAttr;
	unsigned FLi;			// index of the Low heart rate in the spectrum domain (0.666 Hz)
	unsigned FHi;			// Index of the highest heart rate in the spectrum doain (3.333Hz)
	unsigned exit;			// to controll the loop of the measurement
} HeartRate_t;

int HeartRate_init(HeartRate_t *hr);
int HeartRate_runMeasure(HeartRate_t *hr, double *bpm);
void HeartRate_stopMeasure(HeartRate_t *hr);

#endif