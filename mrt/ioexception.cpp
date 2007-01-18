/* M-runtime for c++
 * Copyright (C) 2005-2007 Vladimir Menshakov
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
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

#ifdef WIN32
	strncpy(buf, _strerror(NULL), sizeof(buf));
#else 
	strncpy(buf, strerror(errno), sizeof(buf));
//	if (strerror_r(errno, buf, sizeof(buf)-1) != 0) perror("strerror");
#endif
	return buf;
}

IOException::~IOException() throw() {}
