#ifndef MRT_TIMER_H_
#define MRT_TIMER_H_

#ifdef WIN32
#define WINDOWS_LEAN_AND_MEAN
#include <windows.h>
#endif

namespace sdlx {
class Timer {
public: 
	void reset();
	const int microdelta() const;
	static void microsleep(const int micros);
private: 
#ifdef WIN32
	LARGE_INTEGER tm, freq;
#else	
	timeval tm;
#endif
};
}

#endif
