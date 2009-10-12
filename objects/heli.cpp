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

#include "heli.h"
#include "registrar.h"
#include "alarm.h"
#include "config.h"
#include "zbox.h"
#include "mrt/random.h"
#include "tmx/map.h"
#include "special_owners.h"

Heli::Heli(const std::string &classname) : 
	Object(classname), _fire(false), _alt_fire(false), _left(false) {
	impassability = -1;	
}


void Heli::calculate(const float dt) {
	Object::calculate(dt);

	GET_CONFIG_VALUE("objects.helicopter.rotation-time", float, rt, 0.2);
	limit_rotation(dt, rt, true, true);	
}


void Heli::tick(const float dt) {
	Object::tick(dt);
	
	if (_state.fire && _fire.tick(dt)) {
		_fire.reset();
		if (disable_ai || validateFire(0)) {
			spawn("helicopter-bullet", _left?"helicopter-bullet-left":"helicopter-bullet-right", v2<float>(), _direction);
			_left = !_left;
		}
	}
	if (_state.alt_fire && _alt_fire.tick(dt)) {
		_alt_fire.reset();

		const Matrix<int> & matrix  = Map->get_impassability_matrix(0);
		
		v2<int> pos, pos2;
		get_center_position(pos); 
		v2<int> para_size(64, 64);
		pos -= para_size / 2;
		
		pos2 = pos;
		pos2 += para_size;
		pos2 -= 1;

		const v2<int> tile_size = Map->getTileSize();

		pos /= tile_size;
		pos2 /= tile_size;
		/*
		LOG_DEBUG(("%d %d", matrix.get(pos.y, pos.x), matrix.get(pos.y, pos2.x)));
		LOG_DEBUG(("%d %d", matrix.get(pos2.y, pos.x), matrix.get(pos2.y, pos2.x)));
		*/
		if (matrix.get(pos.y, pos.x) >= 0 || matrix.get(pos.y, pos2.x) >= 0 || 
			matrix.get(pos2.y, pos.x) >= 0 || matrix.get(pos2.y, pos2.x) >= 0) {
			
			Object *o;
			if (_variants.has("kamikazes")) {
				bool gay = mrt::random(6) == 3;
				o = spawn("paratrooper-kamikaze", gay? "gay-paratrooper": "paratrooper");
			} else if (_variants.has("machinegunners")) {
				bool gay = mrt::random(6) == 4;
				o = spawn("paratrooper-machinegunner", gay? "gay-paratrooper": "paratrooper");
			} else if (_variants.has("throwers")) {
				bool gay = mrt::random(6) == 2;
				o = spawn("paratrooper-thrower", gay? "gay-paratrooper": "paratrooper");
			} else {
				o = spawn("bomb", "bomb");
			}
			o->set_z(get_z() - 1, true);
		}
	}
	if (classname == "fighting-vehicle" || classname == "helicopter") {
		int z = get_z();
		int box = ZBox::getBox(z);
		if (box < 1) {
			z += (1 - box) * 2000;
			set_zbox(z);
		}
		//LOG_DEBUG(("box: %d", box));
	} else if (classname == "vehicle") {
		int z = get_z();
		int box = ZBox::getBox(z);
		if (box != 0) {
			z += -box * 2000;
			set_zbox(z);
		}
	}
}

void Heli::on_spawn() {
	if (registered_name.compare(0, 6, "static") == 0)
		remove_owner(OWNER_MAP);

	GET_CONFIG_VALUE("objects.helicopter.fire-rate", float, fr, 0.1f);
	_fire.set(fr);
	
	if (_variants.has("kamikazes") || _variants.has("machinegunners") || _variants.has("throwers")) {
		GET_CONFIG_VALUE("objects.helicopter.disembark-rate", float, br, 1.0f);
		_alt_fire.set(br);
	} else {
		GET_CONFIG_VALUE("objects.helicopter.bombing-rate", float, br, 0.2f);
		_alt_fire.set(br);
	} 

	play("move", true);
}

void Heli::emit(const std::string &event, Object * emitter) {
	if (event == "death") {
		Object * o = spawn("helicorpse", "dead-" + animation);
		o->set_zbox(get_z() - 2000);
	} else if (event == "collision") {
	}
	
	Object::emit(event, emitter);
}

Object* Heli::clone() const  {
	return new Heli(*this);
}

REGISTER_OBJECT("static-helicopter", Heli, ("vehicle"));
