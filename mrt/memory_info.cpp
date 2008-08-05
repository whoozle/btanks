/* M-runtime for c++
 * Copyright (C) 2005-2008 Vladimir Menshakov
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
