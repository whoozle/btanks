#include <assert.h>
#include "resource_manager.h"
#include "object.h"
#include "world.h"
#include "game.h"
#include "shilka.h"

REGISTER_OBJECT("shilka", Shilka, ());

Shilka::Shilka() 
: Object("player"), _fire(0.1, false), _left_fire(true) {
}

Shilka::Shilka(const std::string &animation) 
: Object("player"), _fire(0.1, false), _left_fire(true) {
	setup(animation);
}


void Shilka::onSpawn() {
	Object *_smoke = spawnGrouped("single-pose", "tank-smoke", v3<float>(0,0,0.1), Centered);
	_smoke->impassability = 0;

	add("smoke", _smoke);
}

Object * Shilka::clone() const {
	return new Shilka(*this);
}


void Shilka::emit(const std::string &event, BaseObject * emitter) {
	if (event == "death") {
		LOG_DEBUG(("dead"));
		cancelAll();
		//play("dead", true);
		spawn("corpse", "dead-" + animation, v3<float>(0,0,-0.5), v3<float>(0,0,0));
		_velocity.x = _velocity.y = _velocity.z = 0;
		Object::emit(event, emitter);
	} else if (event == "collision") {
		addDamage(emitter);
	} else if (event == "launch") {
		spawn("rocket", "rocket", v3<float>(0,0,1), _direction);
		const Object * la = ResourceManager.get_const()->getAnimation("rocket-launch");
		v3<float> dpos = (size - la->size).convert<float>();
		dpos.z = 1;
		dpos /= 2;

		Object *o = spawn("rocket-launch", "rocket-launch", dpos, _direction);
		o->setDirection(getDirection());
		//LOG_DEBUG(("dir: %d", o->getDirection()));else Object::emit(event, emitter);
	} else Object::emit(event, emitter);
}



void Shilka::tick(const float dt) {
	Object::tick(dt);

	bool fire_possible = _fire.tick(dt);
	
	if (getState().empty()) {
		play("hold", true);
	}

	_velocity.normalize();
	if (_velocity.is0()) {
		cancelRepeatable();
		play("hold", true);
	} else {
		if (getState() == "hold") {
			cancelAll();
			play("start", false);
			play("move", true);
		}
	}

	if (_state.fire && fire_possible) {
		_fire.reset();
		
		if (getState().substr(0,4) == "fire") 
			cancel();
		
		playNow(_left_fire?"fire-left":"fire-right");
		
		static const std::string left_fire = "shilka-bullet-left";
		static const std::string right_fire = "shilka-bullet-right";
		
		spawn("shilka-bullet", _left_fire?left_fire:right_fire, v3<float>(0,0,-0.1), _direction);
		_left_fire = ! _left_fire;
	}
	
	_state.fire = false;
	
	limitRotation(dt, 8, 0.05, true, false);

	//LOG_DEBUG(("_velocity: %g %g", _velocity.x, _velocity.y));
}

