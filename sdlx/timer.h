#ifndef MRT_TIMER_H_
#define MRT_TIMER_H_

#ifdef WIN32
	union _LARGE_INTEGER;
#	define SDLX_TIMER_USES_QPC

#	ifndef SDLX_TIMER_USES_QPC
#		pragma comment(lib,"winmm.lib")
#	endif
#else 
#	include <time.h>
#endif

#include "export_sdlx.h"

namespace sdlx {
class SDLXAPI Timer {
public: 
	Timer();
	~Timer();

	void reset();
	const int microdelta() const;
	static void microsleep(const int micros);
private: 
#ifdef WIN32
#	ifdef SDLX_TIMER_USES_QPC
	_LARGE_INTEGER *tm, *freq;
#	else
	int tm, res;
#	endif
#else	
	struct timespec tm;
#endif
};
}

#endif
