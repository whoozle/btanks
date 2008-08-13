
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

#include "object.h"
#include "registrar.h"
#include "config.h"
#include "tmx/map.h"

class Mine : public Object {
public:
	Mine() : Object("mine") { piercing = false; pierceable = true; impassability = -1; }
	virtual Object * clone() const;
	virtual void on_spawn();
	virtual void tick(const float dt);
	virtual void emit(const std::string &event, Object * emitter = NULL);
};

void Mine::on_spawn() {
	if (_variants.has("bomberman"))
		disown();
	
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
	if (has_owners() && get_state() == "armed") 
		disown();
	if (get_state() == "armed" && _variants.has("bomberman")) {
		emit("death", NULL);
	}
}

void Mine::emit(const std::string &event, Object * emitter) {
	if (event == "death" && _variants.has("bomberman")) {
		const bool nuke = _variants.has("nuke");
		spawn(nuke?"nuke-explosion":"bomberman-explosion", nuke?"nuke-explosion":"cannon-explosion");
		if (nuke) {
			Object::emit(event, emitter);
			return;
		}

		v2<float> tile_size = Map->getTileSize().convert<float>();
		v2<float> path_tile_size = Map->getPathTileSize().convert<float>();
		//LOG_DEBUG(("tile_size: %g, %g", tile_size.x, tile_size.y));
		
		const Matrix<int>& matrix = get_impassability_matrix();
		for (int d = 0; d < 4; ++d) {
			for(int i = 1; i <= 2; ++i) {
				v2<float> dpos;
				dpos.fromDirection(d, 4);
				dpos *= tile_size * i;
				
				v2<float>tile_pos;
				get_center_position(tile_pos);
				tile_pos += dpos;
				tile_pos /= path_tile_size;
				//LOG_DEBUG(("get(%d, %d) = %d", (int)tile_pos.y, (int)tile_pos.x, matrix.get((int)tile_pos.y, (int)tile_pos.x)));
				if (matrix.get((int)tile_pos.y, (int)tile_pos.x) == -1)
					break;

				spawn("bomberman-explosion", "cannon-explosion", dpos);

				if (matrix.get((int)tile_pos.y, (int)tile_pos.x) < 0)
					break;
				
			}
		}
		Object::emit(event, emitter);	
	} if (event == "collision") {
		if (emitter != NULL && get_state() == "armed") {
			GET_CONFIG_VALUE("objects.regular-mine.triggering-mass", int, m, 20);
			if (emitter->mass < m)
				return;
	
			const bool nuke = _variants.has("nuke");
			
			spawn(nuke?"nuke-explosion":"explosion", nuke?"nuke-explosion":"explosion");
			Object::emit("death", emitter);
			emitter->add_damage(this, max_hp);
		} 
	} else Object::emit(event, emitter);
}


Object* Mine::clone() const  {
	return new Mine(*this);
}

REGISTER_OBJECT("regular-mine", Mine, ());
REGISTER_OBJECT("armed-mine", Mine, ());
