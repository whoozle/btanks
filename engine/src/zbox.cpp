
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
#include "zbox.h"
#include "math/unary.h"
#include <assert.h>

ZBox::ZBox(const v3<int> &position, const v2<int> &size) : position(position), size(size) {}

const bool ZBox::operator<(const ZBox &other) const {
	if (position != other.position)
		return position < other.position;
	if (size != other.size)
		return size < other.size;
	return false;
}

const bool ZBox::in(const v3<int> &p, const bool ignore_z) const {
	if (!ignore_z && getBox(position.z) != getBox(p.z))
		return false;
	return (
		p.x >= position.x && p.y >= position.y && 
		p.x < position.x + size.x && p.y < position.y + size.y
	);
}

const bool ZBox::sameBox(const int z1, const int z2) {
	return getBox(z1) == getBox(z2);	
}

const int ZBox::getBox(const int z) {
	return ((z < 0?z + 1:z) / 1000  + math::sign(z) ) / 2;
}

const int ZBox::getBoxBase(const int z) {
	return getBox(z) * 2000;
}
