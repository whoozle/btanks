#ifndef MRT_TIMESPY_H__
#define MRT_TIMESPY_H__

#include <sys/time.h>
#include <string>
#include "fmt.h"

namespace mrt {
class TimeSpy {
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

