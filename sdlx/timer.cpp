#include "timer.h"
#include "mrt/ioexception.h"

#ifdef WIN32
#include <windows.h>
#endif

#include <time.h>
#include <errno.h>

using namespace sdlx;

void Timer::reset() {
#ifdef WIN32
#error implement me
#else
	if (gettimeofday(&tm, NULL) == -1)
		throw_io(("gettimeofday"));
#endif
}

const int Timer::getTicks() {
#ifdef WIN32
#error implement me
#else
	struct timeval now;
	if (gettimeofday(&now, NULL) == -1)
		throw_io(("gettimeofday"));
	return ((now.tv_sec) * 1000000 + now.tv_usec) % 1000000;
#endif
}


const int Timer::nanodelta() const {
#ifdef WIN32
#error implement me
#else
	struct timeval now;
	if (gettimeofday(&now, NULL) == -1)
		throw_io(("gettimeofday"));
	return (now.tv_sec - tm.tv_sec) * 1000000 + (now.tv_usec - tm.tv_usec);
#endif
}


void Timer::nanosleep(const int nanos) {
#ifdef WIN32
	Sleep(nanos / 1000);
#else 
	struct timespec ts, rem;
	
	ts.tv_sec = 0;
	ts.tv_nsec = nanos;
	do {
		int r = ::nanosleep(&ts, &rem);
		if (r == -1 && errno != EINTR)
			throw_io(("nanosleep"));
		ts = rem;
	} while (rem.tv_nsec == 0 && rem.tv_sec == 0);
#endif
}
