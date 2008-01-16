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


#include "calendar.h"
#include <time.h>

bool mrt::xmas() {
	time_t t;
	time(&t);
	struct tm * d = localtime(&t);
	if (d->tm_mon == 0) {
		//jan
		if (d->tm_mday <= 7) 
			return true;
	} else if (d->tm_mon == 11) {
		//dec
		//xmas eve: 24th of dec
		if (d->tm_mday >= 24) 
			return true;
	}
	return false;
}
