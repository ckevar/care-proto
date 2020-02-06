#ifndef TIME_UTILS_H
#define TIME_UTILS_H 

#include <time.h>

void time_add_ms(struct timespec *t, int ms);
int timecmp(struct timespec t1, struct timespec t2);

#endif