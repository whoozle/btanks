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

#include "rush.h"
#include "tmx/map.h"
#include "object.h"
#include "mrt/random.h"

using namespace ai;

void Rush::calculateW(Way &way, Object *object, const std::string &area) {
	way.clear();
	
	const v2<int> tile_size = Map->getPathTileSize();
	const v2<int> map_size = Map->get_size();
	const Matrix<int> & water = Map->getAreaMatrix(area);
	v2<int> pos;
	object->get_center_position(pos);
	int im = water.get(pos.y / tile_size.y, pos.x / tile_size.x);
	if (im != 1) {
		LOG_WARN(("object %s:%d is now on non-hint area (%d:%d value: %d)", 
			object->animation.c_str(), object->get_id(), pos.y / tile_size.y, pos.x / tile_size.x, im));
		object->emit("death", NULL); //bam! 
		return;
	}
	
	int dirs = object->get_directions_number();
	if (dirs == 1)
		dirs = 16;
	
	int dir = mrt::random(dirs);
	v2<float> d; 
	d.fromDirection(dir, dirs);
	d.normalize((tile_size.x + tile_size.y) / 2);
	int len = 0;
	while(water.get(pos.y / tile_size.y, pos.x / tile_size.x) == 1) {
		++len;
		pos += d.convert<int>();
	}
	//LOG_DEBUG(("d: %g %g, len: %d", d.x, d.y, len));
	len -= (int)(object->size.x + object->size.y) / (tile_size.x + tile_size.y) / 2 + 1;
	if (len > 0) {
		len = 1 + len / 2 + (len % 2) + mrt::random(len / 2);
		object->get_center_position(pos);
		pos += (d * len).convert<int>();
		if (pos.x < object->size.x / 2) 
			pos.x = (int)object->size.x / 2;
		if (pos.y < object->size.y / 2) 
			pos.y = (int)object->size.y / 2;
		if (pos.x + object->size.x / 2 > map_size.x) 
			pos.x = map_size.x - (int)object->size.x / 2;
		if (pos.y + object->size.y / 2 > map_size.y) 
			pos.y = map_size.y - (int)object->size.y / 2;
		way.push_back(pos);
		return;
	}
}
