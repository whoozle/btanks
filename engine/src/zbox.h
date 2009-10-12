#ifndef BTANKS_ZBOX_H__
#define BTANKS_ZBOX_H__

/* Battle Tanks Game
 * Copyright (C) 2006-2009 Battle Tanks team
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

#include "math/v2.h"
#include "math/v3.h"
#include "export_btanks.h"

namespace sdlx {
	class Rect;
}

class BTANKSAPI ZBox {
public: 
	v3<int> position;
	v2<int> size;

	ZBox(const v3<int> &position, const v2<int> &size);
	const bool operator<(const ZBox &other) const;
	const bool in(const v3<int> &position, const bool ignore_z) const;

	static const bool sameBox(const int z1, const int z2);
	static const int getBox(const int z);
	static const int getBoxBase(const int z);
};

#endif
