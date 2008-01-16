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

#include "ioexception.h"
#include <string.h>
#include <errno.h>
#include <stdio.h>

using namespace mrt;

IOException::IOException() {}


const std::string IOException::getCustomMessage() {
	char buf[1024];
	memset(buf, 0, sizeof(buf));

#ifdef _WINDOWS
	strncpy(buf, _strerror(NULL), sizeof(buf));
#else 
	strncpy(buf, strerror(errno), sizeof(buf));
//	if (strerror_r(errno, buf, sizeof(buf)-1) != 0) perror("strerror");
#endif
	return buf;
}

IOException::~IOException() throw() {}
