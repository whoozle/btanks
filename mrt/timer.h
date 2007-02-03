#ifndef MRT_TIMER_H_
#define MRT_TIMER_H_

#if defined(_MSC_VER) || defined(_WINDOWS_)
   #include <time.h>
   #if !defined(_WINSOCK2API_) && !defined(_WINSOCKAPI_)
         struct timeval 
         {
            long tv_sec;
            long tv_usec;
         };
   #endif 
#else
#	include <sys/time.h>
#	include <time.h>
#endif


namespace mrt {
class Timer {
public: 
	void reset();
	const int getTicks();
	const int nanodelta() const;
	static void nanosleep(const int nanos);
private: 
	timeval tm;
};
}

#endif
