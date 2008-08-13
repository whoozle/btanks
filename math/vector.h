#ifndef BTANKS_VECTOR_H__
#define BTANKS_VECTOR_H__

/* Battle Tanks Game
 * Copyright (C) 2006-2008 Battle Tanks team
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

/* 
 * Additional rights can be granted beyond the GNU General Public License 
 * on the terms provided in the Exception. If you modify this file, 
 * you may extend this exception to your version of the file, 
 * but you are not obligated to do so. If you do not wish to provide this
 * exception without modification, you must delete this exception statement
 * from your version and license this file solely under the GPL without exception. 
*/


#include "math/v3.h"
#include <assert.h>

namespace math {

template<typename T>
void getNormalVector(v2<T> &result, const v2<T> &line, const v2<T> &point) {
	if (line.x == 0) {
		assert(line.y != 0);
		result.y = 0;
		result.x = point.x;
		return;
	}
	
	if (line.y == 0) {
		result.x = 0;
		result.y = point.y;
		return;
	}
	
	const T k = line.y / line.x;
	const T b = point.y + point.x / k;
	const T x_cross = b / (k + 1 / k);
	const T y_cross = k * x_cross;
	result.x = point.x - x_cross;
	result.y = point.y - y_cross;
}

}

#endif

