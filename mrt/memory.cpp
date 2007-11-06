#include "memory.h"
#ifdef _WINDOWS
#	include <windows.h>
#else
#	include <unistd.h>
#endif

using namespace mrt;

int MemoryInfo::available() {
#ifdef _WINDOWS
	MEMORYSTATUS mstat;
	GlobalMemoryStatus(&mstat);
	return mstat.dwAvailPhys / 1048576;
#endif

#ifdef _SC_AVPHYS_PAGES
	long page_size = sysconf(_SC_PAGESIZE);
	if (page_size < 0)
		return -1;
	long pages_in_mb = 1024 * 1024 / page_size;
	long mem_avail = sysconf(_SC_AVPHYS_PAGES);
	if (mem_avail < 0)
		return -1;
	//LOG_DEBUG(("%ld pages in 1mb, available memory: %ld", pages_in_mb, mem_avail));
	return mem_avail / pages_in_mb;
#endif	

	return -1;
}
