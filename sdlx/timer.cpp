/* sdlx - c++ wrapper for libSDL
 * Copyright (C) 2005-2007 Vladimir Menshakov
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.

 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
*/

#include "timer.h"
#include "mrt/ioexception.h"

#ifdef _WINDOWS
#	include <windows.h>
#elif defined __APPLE__
#	include <errno.h>
#else
#	include <time.h>
#	include <errno.h>

static clockid_t clock_id = CLOCK_REALTIME;

#endif

using namespace sdlx;

Timer::Timer() {
#ifdef _WINDOWS
#ifdef SDLX_TIMER_USES_QPC
	tm = new LARGE_INTEGER;
	freq = new LARGE_INTEGER;
	if (!QueryPerformanceFrequency(freq)) 
		throw_ex(("QueryPerformanceFrequency failed"));
#elif defined __APPLE__
	//some stupid initialization
#else 
	TIMECAPS caps;
	if (timeGetDevCaps(&caps, sizeof(caps)) != TIMERR_NOERROR) 
		throw_ex(("timeGetDevCaps failed"));
	res = caps.wPeriodMin;
	LOG_DEBUG(("minimum timer's period: %d", res));
#endif
#endif
}

Timer::~Timer() {
#ifdef _WINDOWS
#ifdef SDLX_TIMER_USES_QPC
	delete tm; delete freq;
#endif
#endif
}


void Timer::reset() {
#ifdef _WINDOWS
#	ifdef SDLX_TIMER_USES_QPC
	if (!QueryPerformanceCounter(tm)) 
		throw_ex(("QueryPerformanceCounter failed"));
#	else 
	if (timeBeginPeriod(res) != TIMERR_NOERROR)
		throw_ex(("timeBeginPeriod(%d) failed", res));
	tm = timeGetTime();
	if (timeEndPeriod(res) != TIMERR_NOERROR)
		throw_ex(("timeEndPeriod(%d) failed", res));
#	endif
#elif defined __APPLE__
	if (gettimeofday(&tv, NULL) == -1)
		throw_io(("gettimeofday"));
#else
	if (clock_gettime(clock_id, &tm) != 0)
		throw_io(("clock_gettime"));
#endif
}

const int Timer::microdelta() const {
#ifdef _WINDOWS
#	ifdef SDLX_TIMER_USES_QPC
	LARGE_INTEGER now;
	if (!QueryPerformanceCounter(&now)) 
		throw_ex(("QueryPerformanceCounter failed"));
	//LOG_DEBUG(("%I64d - %I64d = %I64d, freq: %I64d", now.QuadPart, tm->QuadPart, (now.QuadPart - tm->QuadPart), freq->QuadPart));
	return (now.QuadPart - tm->QuadPart) * 1000000 / freq->QuadPart;
#	else
	if (timeBeginPeriod(res) != TIMERR_NOERROR)
		throw_ex(("timeBeginPeriod(%d) failed", res));
	int now = timeGetTime();
	if (timeEndPeriod(res) != TIMERR_NOERROR)
		throw_ex(("timeEndPeriod(%d) failed", res));
	return 1000 * (now - tm);
#	endif
#elif defined __APPLE__
	struct timeval now;
	if (gettimeofday(&now, NULL) == -1)
		throw_io(("gettimeofday"));
	return ((int)now.tv_sec - (int)tv.tv_sec) *1000000 + (int)now.tv_usec - (int)tv.tv_usec;
#else
	struct timespec now;
	if (clock_gettime(clock_id, &now) != 0)
		throw_io(("clock_gettime"));
	return ((int)now.tv_sec - (int)tm.tv_sec) *1000000 + ((int)now.tv_nsec - (int)tm.tv_nsec) / 1000;
#endif
}


void Timer::microsleep(const char *why, const int micros) {
#ifdef _WINDOWS
	timeBeginPeriod(1);
	//LOG_DEBUG(("microsleep('%s', %d)", why, micros));

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
		//LOG_DEBUG(("nanosleep(%s, %u.%u)", why, (unsigned)ts.tv_sec, (unsigned)ts.tv_nsec));
		int r = ::nanosleep(&ts, &rem);
		if (r == 0) 
			return;
		
		if (r == -1 && errno != EINTR)
			throw_io(("nanosleep(%s, %u.%u, %u.%u)", why, (unsigned)ts.tv_sec, (unsigned)ts.tv_nsec, (unsigned)rem.tv_sec, (unsigned)rem.tv_nsec));
		ts = rem;
	} while (rem.tv_nsec != 0 || rem.tv_sec != 0);
#endif
}
