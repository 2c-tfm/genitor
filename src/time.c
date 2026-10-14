#include <time.h>
#include <genitor.h>
#include <unistd.h>

uint64_t time() {
	struct timespec ts;
	clock_gettime(&ts);
	return ts.tv_sec + (ts.tv_nsec / 1000000000.0);
}

int sleep(uint64_t secs) {
    struct timespec req = {
        .tv_sec = secs,
        .tv_nsec = 0
    };

    return nanosleep(&req, 0);
}
