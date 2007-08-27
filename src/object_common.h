#ifndef __BTANKS_OBJECT_COMMON_H__
#define __BTANKS_OBJECT_COMMON_H__

/* Battle Tanks Game
 * Copyright (C) 2006-2007 Battle Tanks team
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


#include "math/v2.h"
#include <deque>
#include <map>

typedef v2<int> WayPoint;
typedef std::deque<WayPoint> Way;

enum GroupType { Fixed, Centered };

class Object;

typedef std::map<const int, Object*> ObjectMap;

#endif

