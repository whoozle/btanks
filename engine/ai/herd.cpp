
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

#include "herd.h"
#include <set>
#include "world.h"
#include "object.h"
#include "config.h"
#include "zbox.h"
#include "tmx/map.h"

void ai::Herd::calculateV(v2<float> &velocity, Object *sheep, const int leader, const float distance) {
	bool was_stopped = velocity.is0();
	velocity.clear();
	
	std::set<const Object *> o_set;
	World->enumerate_objects(o_set, sheep, distance, NULL);
	int n = 0;
	for(std::set<const Object *>::iterator i = o_set.begin(); i != o_set.end(); ++i) {
		const Object *o = *i;
		if (o->impassability == 0 || (leader && o->get_summoner() != leader)) 
			continue;
		int cd = getComfortDistance(o);
		if (cd == -1)
			continue;
			
		v2<float> pos = sheep->get_relative_position(o);
		float r = pos.length();
		if (r < 0.001) r = 0.001;
		if (pos.quick_length() < cd * cd)
			velocity -= pos / r;
		else 
			velocity += pos / r;
		
		++n;
	}
	const v2<int> tile_size = Map->getPathTileSize();
	v2<int> pos = sheep->get_center_position().convert<int>() / tile_size;
	
	const Matrix<int> &hint = Map->getAreaMatrix(sheep->registered_name);

	GET_CONFIG_VALUE("objects.ai.hint-gravity", float, hgc, 10.0f);
	v2<int> size = v2<int>((int)distance, (int)distance * 4 / 3) / tile_size / 2;
	for(int y = -size.y; y <= size.y; ++y) 
		for(int x = -size.x; x < size.x; ++x) {
			if (hint.get(pos.y + y, pos.x + x)) {
				v2<float> dpos(x, y);
				//LOG_DEBUG(("%d:%s %g %g, %g", sheep->get_id(), sheep->registered_name.c_str(), dpos.x, dpos.y, dpos.length()));
				float r = dpos.normalize();
				velocity += dpos * hgc / r;
			}
		}
		
	const Object * o = leader?World->getObjectByID(leader): NULL;
	if (o != NULL && !ZBox::sameBox(o->get_z(), sheep->get_z())) 
		o = NULL;
	
	if (o != NULL) {
		//LOG_DEBUG(("leader: %p", o));
		v2<float> pos = sheep->get_relative_position(o);
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

