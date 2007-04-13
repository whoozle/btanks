#ifndef MRT_TIMESPY_H__
#define MRT_TIMESPY_H__

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

#include <string>
#include "fmt.h"
#include "export.h"

namespace mrt {
class MRTAPI TimeSpy {
public: 
	TimeSpy(const std::string &message);
	~TimeSpy();
private: 
	TimeSpy(const TimeSpy&);
	const TimeSpy& operator=(const TimeSpy&);
	
	std::string message;
	struct timeval tm;
};
}

#define MRT_CONCATENATE(x, y) CONCATENATE_DIRECT(x, y) 
#define MRT_CONCATENATE_DIRECT(x, y) x##y
#define TIMESPY(str) mrt::TimeSpy CONCATENATE(spy, __LINE__) ( mrt::formatString str );

#endif

