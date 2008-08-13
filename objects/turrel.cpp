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
#include "alarm.h"
#include "registrar.h"
#include "config.h"
#include "mrt/random.h"
#include "ai/base.h"

class Turrel : public Object, protected ai::Base {
public:
	Turrel(const std::string &classname) : 
		Object(classname), _reaction(true), _fire(true), _left(false) { impassability = 1; set_directions_number(8); }
	
	virtual Object * clone() const { return new Turrel(*this); }
	virtual void on_spawn();
	virtual void tick(const float dt);
	virtual void calculate(const float dt);
	virtual void emit(const std::string &event, Object * emitter = NULL);
	virtual const bool take(const BaseObject *obj, const std::string &type);
	virtual const std::string getType() const { return "machinegunner"; }
	virtual const int getCount() const { return -1; }

	virtual void serialize(mrt::Serializator &s) const;
	virtual void deserialize(const mrt::Serializator &s); 

private: 
	Alarm _reaction, _fire;
	bool _left;
};

void Turrel::on_spawn() {
	play("hold", true);

	float fr;
	Config->get("objects." + registered_name + ".fire-rate", fr, 0.1);
	_fire.set(fr);

	GET_CONFIG_VALUE("objects.turrel.reaction-time", float, rt, 0.2);
	mrt::randomize(rt, rt / 10);
	_reaction.set(rt);

	ai::Base::multiplier = 5.0f;
	ai::Base::on_spawn(this);
}

void Turrel::tick(const float dt) {
	Object::tick(dt);
	bool ai = (_parent != NULL)? !_parent->disable_ai:true;
	//LOG_DEBUG(("ai: %s, _parent: %s, parent->disable_ai : %s", ai?"true":"false", _parent?_parent->animation.c_str():"-", _parent?(_parent->disable_ai?"true":"false"):"-"));
	if (_fire.tick(dt) && _state.fire && (!ai || canFire())) {
		bool air_mode = (_parent != NULL)?_parent->get_player_state().alt_fire:true;
		cancel_all();
		play(_left? "fire-left": "fire-right", false);
		play("hold", true);
		std::string animation = mrt::format_string("buggy-%s-%s", air_mode?"air-bullet":"bullet", _left?"left":"right");
		Object *bullet = (_parent == NULL? this: _parent)->spawn("buggy-bullet", animation, v2<float>(), _direction);
		
		bullet->set_z(air_mode? bullet->get_z() + 2000:get_z() - 1, true);
		_left = !_left;
	}
}

void Turrel::calculate(const float dt) {
	if (!_reaction.tick(dt))
		return;

	static std::set<std::string> targets;
	if (targets.empty()) {
		//targets.insert("missile");
		targets.insert("fighting-vehicle");
		targets.insert("trooper");
		targets.insert("cannon");
		targets.insert("kamikaze");
		targets.insert("boat");
		targets.insert("helicopter");
		targets.insert("monster");
		targets.insert("watchtower");
		targets.insert("paratrooper");
	}
	
	bool air_mode = (_parent != NULL)?_parent->get_player_state().alt_fire:true;
	if (air_mode || _variants.has("ground-aim")) {
		v2<float> pos, vel;
		int z0 = get_z();

		if (air_mode) {
			set_z(z0 + 2000, true); //temporary move up turrel %) hack for air mode :)
		}

		if (get_nearest(targets, getWeaponRange("buggy-bullet"), pos, vel, true)) {
			_direction = pos;
			_state.fire = true;
			_direction.quantize8();
			set_direction(_direction.get_direction8() - 1);
		} else {
			_state.fire = false;
		}

		if (air_mode) {
			set_z(z0, true);
		}
	} else {
		if (_parent != NULL) {
			_state.fire = _parent->get_player_state().fire;

			int idx = _parent->get_direction();
			set_direction(idx);
			_direction.fromDirection(idx, get_directions_number());
		}
	}
}

void Turrel::emit(const std::string &event, Object * emitter) {
	if (event == "hold" || event == "move") {
		cancel_all();
		play(event, true);
		return;
	}
	Object::emit(event, emitter);
}

const bool Turrel::take(const BaseObject *obj, const std::string &type) {
	return false;
}

void Turrel::serialize(mrt::Serializator &s) const {
	Object::serialize(s);
	ai::Base::serialize(s);
	s.add(_reaction);
	s.add(_fire);
	s.add(_left);
}

void Turrel::deserialize(const mrt::Serializator &s) {
	Object::deserialize(s);
	ai::Base::deserialize(s);
	s.get(_reaction);
	s.get(_fire);
	s.get(_left);	
}


REGISTER_OBJECT("turrel", Turrel, ("turrel"));
REGISTER_OBJECT("turrel-on-buggy", Turrel, ("turrel"));
