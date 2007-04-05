
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

#include "object.h"
#include "resource_manager.h"
#include "config.h"
#include "tmx/map.h"

class Mine : public Object {
public:
	Mine() : Object("mine") { piercing = false; pierceable = true; }
	virtual Object * clone() const;
	virtual void onSpawn();
	virtual void tick(const float dt);
	virtual void emit(const std::string &event, Object * emitter = NULL);
};

void Mine::onSpawn() {
	if (registered_name != "armed-mine") {
		play("3", false);
		play("pause", false);
		play("2", false);
		play("pause", false);
		play("1", false);
		play("pause", false);
	}
	play("armed", true);
}

void Mine::tick(const float dt) {
	Object::tick(dt);
	if (hasOwners() && getState() == "armed") 
		disown();
	if (getState() == "armed" && registered_name == "bomberman-mine") {
		v2<float> tile_size = Map->getTileSize().convert<float>();
		v2<float> path_tile_size = Map->getPathTileSize().convert<float>();
		LOG_DEBUG(("tile_size: %g, %g", tile_size.x, tile_size.y));
		spawn("bomberman-explosion", "cannon-explosion");
		const Matrix<int>& matrix = Map->getImpassabilityMatrix();
		for (int d = 0; d < 4; ++d) {
			for(int i = 1; i <= 2; ++i) {
				v2<float> dpos;
				dpos.fromDirection(d, 4);
				dpos *= tile_size * i;
				
				v2<float>tile_pos;
				getCenterPosition(tile_pos);
				tile_pos += dpos;
				tile_pos /= path_tile_size;
				LOG_DEBUG(("get(%d, %d) = %d", (int)tile_pos.y, (int)tile_pos.x, matrix.get((int)tile_pos.y, (int)tile_pos.x)));
				if (matrix.get((int)tile_pos.y, (int)tile_pos.x) == -1)
					break;
				
				spawn("bomberman-explosion", "cannon-explosion", dpos);
			}
		}
		emit("death", NULL);
	}
}

void Mine::emit(const std::string &event, Object * emitter) {
	if (event == "collision") {
		if (emitter != NULL && getState() == "armed") {
			GET_CONFIG_VALUE("objects.regular-mine.triggering-mass", int, m, 20);
			if (emitter->mass < m)
				return;
			
			spawn("explosion", "explosion");
			Object::emit("death", emitter);
			emitter->addDamage(this, max_hp);
		} 
	} else Object::emit(event, emitter);
}


Object* Mine::clone() const  {
	return new Mine(*this);
}

REGISTER_OBJECT("bomberman-mine", Mine, ());
REGISTER_OBJECT("regular-mine", Mine, ());
REGISTER_OBJECT("armed-mine", Mine, ());
