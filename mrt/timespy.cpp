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


#include "timespy.h"
#include "ioexception.h"
#include "logger.h"


using namespace mrt;

#if defined(_MSC_VER) || defined(_WINDOWS_) || defined(_WINDOWS)
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
	
	LOG_DEBUG(("%s: %ld mcs", message.c_str(), (now.tv_sec - tm.tv_sec) * 1000000 + (now.tv_usec - tm.tv_usec)));
}
