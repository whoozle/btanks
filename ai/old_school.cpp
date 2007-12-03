
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

#include "old_school.h"
#include <set>
#include "world.h"
#include "object.h"
#include "mrt/random.h"
#include "tmx/map.h"

ai::OldSchool::OldSchool() {}

void ai::OldSchool::onSpawn(Object *object) {
	trottle = 0;
}

void ai::OldSchool::calculateV(v2<float> &velocity, Object *object) {
	if (object->isDriven())
		return;

	++trottle;
	if (trottle < 10) {
		return;
	} else {
		trottle = 0;
	}

	int dirs = object->getDirectionsNumber();

	int action = mrt::random(2);

	if (action == 0) {
		int dir = mrt::random(dirs);
		object->setDirection(dir);
		velocity.clear();
	} else if (action == 1) {
		int dir = mrt::random(dirs);
		v2<int> pos;
		object->getCenterPosition(pos);
		v2<int> tile_size = Map->getPathTileSize();
	
		const Matrix<int> &matrix = Map->getImpassabilityMatrix(object->getZ());
	
		v2<float> dpos;
		dpos.fromDirection(dir, dirs);
		pos += (dpos * tile_size.convert<float>() * 2).convert<int>();
	
		pos /= tile_size;
		if (matrix.get(pos.y, pos.x) != -1) {
			Way way;
			way.push_back(pos * tile_size + tile_size / 2);
			object->setWay(way);
		}
	}
}


void ai::OldSchool::serialize(mrt::Serializator &s) const {
	s.add(trottle);
}

void ai::OldSchool::deserialize(const mrt::Serializator &s) {
	s.get(trottle);	
}
