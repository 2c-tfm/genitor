#ifndef TIME_H
#define TIME_H

#include <types.h>

#define CLOCK_REALTIME	0
#define CLOCK_MONOTONIC	1

struct timespec {
	long tv_sec;
	long tv_nsec;
};

long clock_gettime(struct timespec *ts);
uint64_t time();
int sleep(uint64_t secs);

#endif
