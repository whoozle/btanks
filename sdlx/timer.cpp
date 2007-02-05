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
	if (!QueryPerformanceFrequency(&freq)) 
		throw_ex(("QueryPerformanceFrequency failed"));

	if (!QueryPerformanceCounter(&tm)) 
		throw_ex(("QueryPerformanceCounter failed"));
#else
	if (gettimeofday(&tm, NULL) == -1)
		throw_io(("gettimeofday"));
#endif
}

const int Timer::microdelta() const {
#ifdef WIN32
	LARGE_INTEGER now;
	if (!QueryPerformanceCounter(&now)) 
		throw_ex(("QueryPerformanceCounter failed"));
	
	return (now.QuadPart - tm.QuadPart) * 1000000 / freq.QuadPart;
#else
	struct timeval now;
	if (gettimeofday(&now, NULL) == -1)
		throw_io(("gettimeofday"));
	return (now.tv_sec - tm.tv_sec) *1000000 + (now.tv_usec - tm.tv_usec) / 1000;
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
