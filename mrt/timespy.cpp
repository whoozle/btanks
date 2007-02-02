#include "timespy.h"
#include "ioexception.h"
#include "logger.h"

#include <time.h>

using namespace mrt;

TimeSpy::TimeSpy(const std::string &message): message(message) {
	if (gettimeofday(&tm, NULL) == -1)
		throw_io(("gettimeofday"));
}

TimeSpy::~TimeSpy() {
	struct timeval now;
	if (gettimeofday(&now, NULL) == -1)
		throw_io(("gettimeofday"));
	
	LOG_DEBUG(("%s: %ld ns", message.c_str(), (now.tv_sec - tm.tv_sec) * 1000000 + (now.tv_usec - tm.tv_usec)));
}
