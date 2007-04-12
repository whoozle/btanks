
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

#include "herd.h"
#include <set>
#include "world.h"
#include "object.h"
#include "config.h"
#include "mrt/random.h"
#include "zbox.h"
#include "tmx/map.h"

void ai::Herd::calculateV(v2<float> &velocity, Object *sheep, const int leader, const float distance) {
	bool was_stopped = velocity.is0();
	velocity.clear();
	
	std::set<const Object *> o_set;
	World->enumerateObjects(o_set, sheep, distance, NULL);
	int n = 0;
	for(std::set<const Object *>::iterator i = o_set.begin(); i != o_set.end(); ++i) {
		const Object *o = *i;
		if (leader && o->getSummoner() != leader) 
			continue;
		int cd = getComfortDistance(o);
		if (cd == -1)
			continue;
			
		v2<float> pos = sheep->getRelativePosition(o);
		float r = pos.length();
		if (r < 0.001) r = 0.001;
		if (pos.quick_length() < cd * cd)
			velocity -= pos / r;
		else 
			velocity += pos / r;
		
		++n;
	}
	const v2<int> tile_size = Map->getPathTileSize();
	v2<float> pos, vel;
	sheep->getInfo(pos, vel);
	pos /= tile_size.convert<float>();
	
	const Matrix<int> &hint = Map->getAreaMatrix(sheep->registered_name);
	int w = hint.getWidth(), h = hint.getHeight();
	for(int y = 0; y < h; ++y) 
		for(int x = 0; x < w; ++x) {
			if (hint.get(y, x)) {
				float r = pos.distance(v2<float>(x, y));
				velocity += 1 / r;
			}
		}
		
	const Object * o = leader?World->getObjectByID(leader): NULL;
	if (o != NULL && !ZBox::sameBox(o->getZ(), sheep->getZ())) 
		o = NULL;
	
	if (o != NULL) {
		//LOG_DEBUG(("leader: %p", o));
		v2<float> pos = sheep->getRelativePosition(o);
		int cd = getComfortDistance(NULL);
		if (pos.quick_length() < cd * cd)
			velocity -= pos;
		else 
			velocity += pos * n;
	}
	float v = velocity.normalize();

	if (v < (was_stopped?0.5:0.0001)) 
		velocity.clear();
}

