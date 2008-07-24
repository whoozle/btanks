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

#include "destructable_object.h"
#include "registrar.h"
#include "config.h"
#include "animation_model.h"
#include "zbox.h"

DestructableObject::DestructableObject(const std::string &classname) : 
		Object(classname), 
		_broken(false),
		_respawn(false) {}

void DestructableObject::serialize(mrt::Serializator &s) const {
	Object::serialize(s);
	s.add(_broken);
	s.add(_respawn);
}

void DestructableObject::deserialize(const mrt::Serializator &s) {
	Object::deserialize(s);
	s.get(_broken);
	s.get(_respawn);
}

void DestructableObject::onBreak() {
}


void DestructableObject::destroy() {
	if (_broken)
		return;
	
		_broken = true;
		hp = -1;
		if (_variants.has("make-pierceable"))
			pierceable = true;
		cancel_all();
		play("fade-out", false); 
		play("broken", true);
		classname = "debris";
		
		if (_variants.has("with-fire")) {
			const AnimationModel *model = get_animation_model();
			int my_z = get_z();
			if (model != NULL) {
				const Pose * pose = model->getPose("broken");
				if (pose != NULL && pose->z > -10000)
					my_z = pose->z + ZBox::getBoxBase(my_z);
			}
			Object *fire = spawn("fire", "fire");
			if (my_z > fire->get_z())
				fire->set_z(my_z + 1, true);
		}

		if (_variants.has("respawning")) {
			GET_CONFIG_VALUE("objects." + registered_name + ".respawn-interval", float, ri, 20.0f);
			_respawn.set(ri);
		}

		onBreak();
}


void DestructableObject::add_damage(Object *from, const int dhp, const bool emitDeath) {
	if (_broken)
		return;

	Object::add_damage(from, dhp, false);
	if (hp <= 0) {
		destroy();
	}
}

void DestructableObject::emit(const std::string &event, Object * emitter) {
	if (event == "destroy") {
		destroy();
	} else Object::emit(event, emitter);
}

void DestructableObject::tick(const float dt) {
	Object::tick(dt);
	const std::string& state = get_state();
	if (state.empty()) {
		//LOG_DEBUG(("over"));
		emit("death", this);
	}
	if (_broken && _variants.has("respawning") && _respawn.tick(dt)) {
		//fixme: restore classname
		LOG_DEBUG(("repairing..."));
		hp = max_hp;
		_broken = false;
		cancel_all();
		on_spawn();
		
		if (_variants.has("make-pierceable"))
			pierceable = false;
	}
}

void DestructableObject::on_spawn() {
	play("main", true);
	if (get_state().empty())
		throw_ex(("%s:%s does not have initial pose ('main')", registered_name.c_str(), animation.c_str()));
}


Object* DestructableObject::clone() const  {
	return new DestructableObject(*this);
}

REGISTER_OBJECT("destructable-object", DestructableObject, ("destructable-object"));
