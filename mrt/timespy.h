#ifndef MRT_TIMESPY_H__
#define MRT_TIMESPY_H__

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

#if defined(_MSC_VER) || defined(_WINDOWS_) || defined(_WINDOWS)
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
#include "export_mrt.h"

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

