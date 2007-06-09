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
#ifdef WIN32
	tm = new LARGE_INTEGER;
	freq = new LARGE_INTEGER;
#endif
}

Timer::~Timer() {
#ifdef WIN32
	delete tm; delete freq;
#endif
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
	timeBeginPeriod(1);

	/*
	LARGE_INTEGER t1, t2, freq;

	bool done = false;
	QueryPerformanceFrequency(&freq);
	QueryPerformanceCounter(&t1);
	do {
		QueryPerformanceCounter(&t2);
                
		int ticks_passed = (int)((__int64)(t2.QuadPart) - (__int64)(t1.QuadPart));
		int ticks_left = micros - ticks_passed;

		if (t2.QuadPart < t1.QuadPart)    // time wrap
			done = true;

        if (ticks_passed >= micros)
        	done = true;
                
		if (!done) {
			// if > 0.002s left, do Sleep(1), which will actually sleep some 
            //   steady amount, probably 1-2 ms,
            //   and do so in a nice way (cpu meter drops; laptop battery spared).
            // otherwise, do a few Sleep(0)'s, which just give up the timeslice,
            //   but don't really save cpu or battery, but do pass a tiny
            //   amount of time.
            if (ticks_left > (int)(freq.QuadPart) * 2 / 1000)
     			Sleep(1);
            else                        
               //for (int i=0; i < 10; i++) 
                            Sleep(0);  // causes thread to give up its timeslice
		}
	} while (!done);        
	*/
	Sleep(micros / 1000);
	
	timeEndPeriod(1);
#else 
	struct timespec ts, rem;
	
	ts.tv_sec = micros / 1000000;
	ts.tv_nsec = (micros % 1000000) * 1000;
	
	do {
		//LOG_DEBUG(("nanosleep(%u.%u)", (unsigned)ts.tv_sec, (unsigned)ts.tv_nsec));
		int r = ::nanosleep(&ts, &rem);
		if (r == 0) 
			return;
		
		if (r == -1 && errno != EINTR)
			throw_io(("nanosleep(%u.%u, %u.%u)", (unsigned)ts.tv_sec, (unsigned)ts.tv_nsec, (unsigned)rem.tv_sec, (unsigned)rem.tv_nsec));
		ts = rem;
	} while (rem.tv_nsec != 0 || rem.tv_sec != 0);
#endif
}
