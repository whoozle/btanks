#include "resource_manager.h"
#include "object.h"
#include "world.h"
#include "game.h"
#include "launcher.h"
#include "config.h"

REGISTER_OBJECT("launcher", Launcher, ());

Launcher::Launcher() 
: Object("player"), _fire(false) {
}

Launcher::Launcher(const std::string &animation) 
: Object("player"), _fire(false) {
	setup(animation);
}

Object * Launcher::clone() const {
	return new Launcher(*this);
}

void Launcher::onSpawn() {
	Object *_smoke = spawnGrouped("single-pose", "launcher-smoke", v3<float>::empty, Centered);
	_smoke->hp = 100000;
	_smoke->impassability = 0;
	add("smoke", _smoke);
	Object *_missiles = spawnGrouped("missiles-on-launcher", "guided-missiles-on-launcher", v3<float>::empty, Centered);
	_missiles->hp = 100000;
	_missiles->impassability = 0;
	add("missiles", _missiles);
	
	GET_CONFIG_VALUE("objects.launcher.fire-rate", float, fr, 0.3);
	_fire.set(fr);
}


void Launcher::emit(const std::string &event, BaseObject * emitter) {
	if (event == "death") {
		LOG_DEBUG(("dead"));
		cancelAll();
		//play("dead", true);
		spawn("corpse", "dead-" + animation);
		_velocity.x = _velocity.y = _velocity.z = 0;

		Object::emit(event, emitter);
	} else if (event == "launch") {
		v3<float> v = _velocity.is0()?_direction:_velocity;
		v.normalize();
		spawn("guided-missile", "guided-missile", v3<float>::empty, v);
		const Object * la = ResourceManager.get_const()->getAnimation("missile-launch");
		v3<float> dpos = (size - la->size).convert<float>();
		dpos.z = 0;
		dpos /= 2;

		Object *o = spawn("missile-launch", "missile-launch", dpos, _direction);
		o->setDirection(getDirection());
		//LOG_DEBUG(("dir: %d", o->getDirection()));
	} else Object::emit(event, emitter);
}


void Launcher::calculate(const float dt) {
	BaseObject::calculate(dt);
	GET_CONFIG_VALUE("objects.launcher.rotation-time", float, rt, 0.07);
	limitRotation(dt, 8, rt, true, false);
	//LOG_DEBUG(("_velocity: %g %g", _velocity.x, _velocity.y));
}


void Launcher::tick(const float dt) {
	Object::tick(dt);

	bool fire_possible = _fire.tick(dt);
	
	if (getState().empty()) {
		play("hold", true);
		groupEmit("missiles", "hold");
	}

	if (_velocity.is0()) {	
		cancelRepeatable();
		play("hold", true);
		groupEmit("missiles", "hold");
	} else {
		if (getState() == "hold") {
			cancelAll();
			//play("start", false);
			play("move", true);
			groupEmit("missiles", "move");
		}
	}

	if (_state.alt_fire && fire_possible) {
		_fire.reset();
		groupEmit("missiles", "launch");
	}
}

const bool Launcher::take(const BaseObject *obj, const std::string &type) {
	if (get("missiles")->take(obj, type))
		return true;
	return BaseObject::take(obj, type);
}

void Launcher::serialize(mrt::Serializator &s) const {
	Object::serialize(s);
	_fire.serialize(s);
}

void Launcher::deserialize(const mrt::Serializator &s) {
	Object::deserialize(s);
	_fire.deserialize(s);
}
