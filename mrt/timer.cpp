#include "timer.h"
#include "mrt/ioexception.h"

#ifdef WIN32
#include <windows.h>
#endif

#include <time.h>
#include <errno.h>

using namespace mrt;

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
	} while (rem.tv_nsec == 0 && rem.tv_sec == 0);
#endif
}
