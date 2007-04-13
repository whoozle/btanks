#ifndef MRT_TIMER_H_
#define MRT_TIMER_H_

#ifdef WIN32
	union _LARGE_INTEGER;
#else 
#	include <time.h>
#endif

#include "export.h"

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
	_LARGE_INTEGER *tm, *freq;
#else	
	struct timespec tm;
#endif
};
}

#endif
