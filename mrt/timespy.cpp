#include "timespy.h"
#include "ioexception.h"
#include "logger.h"


using namespace mrt;

#if defined(_MSC_VER) || defined(_WINDOWS_)
   static int gettimeofday(struct timeval* tp, void* tzp) 
   {
/*      DWORD t;
      t = timeGetTime();
      tp->tv_sec = t / 1000;
      tp->tv_usec = t % 1000;
      return 0;
*/
		throw_ex(("unimplemented"));
   }
#endif


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
