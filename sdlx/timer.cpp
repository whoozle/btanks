#include "timer.h"
#include "mrt/ioexception.h"

#ifdef WIN32
#	include <windows.h>
#else
#	include <time.h>
#	include <errno.h>

static clockid_t clock_id = CLOCK_REALTIME;

#endif

using namespace sdlx;

Timer::Timer() {
	tm = new LARGE_INTEGER;
	freq = new LARGE_INTEGER;
}

Timer::~Timer() {
	delete tm; delete freq;
}


void Timer::reset() {
#ifdef WIN32
	if (!QueryPerformanceFrequency(freq)) 
		throw_ex(("QueryPerformanceFrequency failed"));

	if (!QueryPerformanceCounter(tm)) 
		throw_ex(("QueryPerformanceCounter failed"));
#else
	if (clock_gettime(clock_id, &tm) != 0)
		throw_io(("clock_gettime"));
#endif
}

const int Timer::microdelta() const {
#ifdef WIN32
	LARGE_INTEGER now;
	if (!QueryPerformanceCounter(&now)) 
		throw_ex(("QueryPerformanceCounter failed"));
	
	return (now.QuadPart - tm->QuadPart) * 1000000 / freq->QuadPart;
#else
	struct timespec now;
	if (clock_gettime(clock_id, &now) != 0)
		throw_io(("clock_gettime"));
	return (now.tv_sec - tm.tv_sec) *1000000 + (now.tv_nsec - tm.tv_nsec) / 1000;
#endif
}


void Timer::microsleep(const int micros) {
#ifdef WIN32
	Sleep(micros);	
#else 
	struct timespec ts, rem;
	
	ts.tv_sec = 0;
	ts.tv_nsec = micros * 1000;
	do {
		int r = ::nanosleep(&ts, &rem);
		if (r == -1 && errno != EINTR)
			throw_io(("nanosleep"));
		ts = rem;
	} while (rem.tv_nsec == 0 && rem.tv_sec == 0);
#endif
}
