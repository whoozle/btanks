#include <assert.h>
#include "resource_manager.h"
#include "object.h"
#include "world.h"
#include "game.h"
#include "shilka.h"
#include "config.h"

REGISTER_OBJECT("shilka", Shilka, ());

Shilka::Shilka() 
: Object("player"), _fire(false), _dirt_fire(false), _left_fire(true) {
}

Shilka::Shilka(const std::string &animation) 
: Object("player"), _fire(false), _dirt_fire(false), _left_fire(true) {
	setup(animation);
}


void Shilka::onSpawn() {
	Object *_smoke = spawnGrouped("single-pose", "tank-smoke", v3<float>(0,0,0.1), Centered);
	_smoke->impassability = 0;

	add("smoke", _smoke);
	
	GET_CONFIG_VALUE("objects.shilka.fire-rate", float, fr, 0.2);
	_fire.set(fr);

	GET_CONFIG_VALUE("objects.shilka.special-fire-rate", float, sfr, 0.7);
	_dirt_fire.set(sfr);
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
	} else Object::emit(event, emitter);
}


void Shilka::calculate(const float dt) {
	BaseObject::calculate(dt);	
	GET_CONFIG_VALUE("objects.shilka.rotation-time", float, rt, 0.05);
	limitRotation(dt, 8, rt, true, false);

	//LOG_DEBUG(("_velocity: %g %g", _velocity.x, _velocity.y));
}


void Shilka::tick(const float dt) {
	Object::tick(dt);

	bool fire_possible = isEffectActive("dirt")?(_fire.tick(dt),_dirt_fire.tick(dt)):(_dirt_fire.tick(dt),_fire.tick(dt));
	
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
		_dirt_fire.reset();
		
		if (getState().substr(0,4) == "fire") 
			cancel();
		
		playNow(_left_fire?"fire-left":"fire-right");
		
		static const std::string left_fire = "shilka-bullet-left";
		static const std::string right_fire = "shilka-bullet-right";
		std::string animation = "shilka-";
		animation += isEffectActive("dirt")?"dirt-":"";
		animation += "bullet-";
		animation += (_left_fire)?"left":"right";
		
		spawn(isEffectActive("dirt")?"dirt-bullet":"shilka-bullet", animation, v3<float>(0,0,-0.1), _direction);
		_left_fire = ! _left_fire;
	}
}

const bool Shilka::take(const BaseObject *obj, const std::string &type) {
	if (obj->classname == "effects") {
		addEffect(type);
		return true;
	}
	return BaseObject::take(obj, type);
}

