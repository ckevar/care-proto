#include "timeutils.h"

void time_add_ms(struct timespec *t, int ms) {
	t->tv_sec += ms/1000;
	t->tv_nsec += (ms % 1000) * 1000000;
	if(t->tv_nsec > 1000000000) {
		t->tv_nsec -= 1000000000;
		t->tv_sec += 1;
	}
}

int timecmp(struct timespec t1, struct timespec t2) {
	if(t1.tv_sec > t2.tv_sec) return 1;
	if(t1.tv_sec < t2.tv_sec) return -1;
	if(t1.tv_nsec > t2.tv_nsec) return 1;
	if(t1.tv_nsec < t2.tv_nsec) return -1;
	return 0;
}