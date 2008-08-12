#include "object.h"
#include "registrar.h"
#include "mrt/random.h"
#include "config.h"
#include "world.h"
#include "math/unary.h"
#include "tmx/map.h"
#include "ai/targets.h"

class BallisticMissile : public Object {
public: 
	virtual Object * clone() const { return new BallisticMissile(*this); }
	
	BallisticMissile() : Object("ballistic-missile"), _fall(false), _launch(false), _reaction(true) {
		set_directions_number(16);
		piercing = true;
	}
	
	void on_spawn() {
		play("main", true);
		float duration = 5.0f, launch_duration = 512.0f / speed;
		_launch.set(launch_duration);
		_fall.set(duration - launch_duration);
		
		float reaction = 0.05f;
		mrt::randomize(reaction, reaction / 10);
		_reaction.set(reaction);
		
		set_direction(4);
		_direction = _velocity = v2<float>(0, -1);

		Object *target = spawn("ballistic-missile-target", "target");
		target_id = target->get_id();
		speed_backup = speed;
	}
	
	virtual const bool skip_rendering() const {
		float l = _launch.get(), f = _fall.get();
		return l >= 1 && f < 1;
	}
	
	void calculate(const float dt) {
		bool react = _reaction.tick(dt), falling = _fall.tick(dt), launch = !_launch.tick(dt);
		//LOG_DEBUG(("launch: %c, falling: %c", launch?'+':'-', falling?'+':'-'));
		if (launch) {
			_velocity = v2<float>(0, -1);
		} else if (!falling) {
			v2<float> pos = get_position();
			if (react) {
				Object *target = World->getObjectByID(target_id);
				if (target == NULL) {
					Object::emit("death", NULL); //just hide 
					return;
				}
				speed = target->speed * 1.3f;
				_velocity = get_relative_position(target) + v2<float>(0, -512);
				//LOG_DEBUG(("correcting: %g", _velocity.x));
			}
		} else { //falling
			if (speed != speed_backup) {
				speed = speed_backup;
				Object *target = World->getObjectByID(target_id);
				ttl = ((target != NULL)?get_relative_position(target).length():512.0f) / speed;
				set_direction(12);
			}
			_velocity = v2<float>(0, 1);
			/*
			
			v2<float> pos = get_center_position(), tpos;

			if (target != NULL)			
				tpos = target->get_center_position();
			
			if (target == NULL || tpos.y <= 0) {
				tpos = pos + v2<float>(0, 50);
			}
			
			_velocity = Map->distance(pos, tpos);
			if (_velocity.y <= 0) {
				if (animation == "nuke-missile") {
					spawn("nuke-explosion", "nuke-explosion");
				}
			
				emit("death", NULL);
				if (target) 
					target->emit("death", NULL);
			} else {
				if (math::abs(_velocity.x) * 5 > _velocity.y)
					_velocity.x = _velocity.y / 5;
			}
			*/
		}
	}
	void emit(const std::string &event, Object * emitter) {
		if (event == "death") {
			Object *target = World->getObjectByID(target_id);
			if (target != NULL)
				target->emit("death", NULL);
			if (animation == "nuke-missile") {
				spawn("nuke-explosion", "nuke-explosion");
			}
		}
		Object::emit(event, emitter);
	}

	virtual void serialize(mrt::Serializator &s) const {
		Object::serialize(s);
		s.add(_fall);
		s.add(_launch);
		s.add(_reaction);
		s.add(speed_backup);
		s.add(target_id);
	}

	virtual void deserialize(const mrt::Serializator &s) {
		Object::deserialize(s);
		s.get(_fall);
		s.get(_launch);
		s.get(_reaction);
		s.get(speed_backup);
		s.get(target_id);
	}
private: 
	Alarm _fall, _launch, _reaction;
	float speed_backup;
	int target_id;
};

class BallisticMissileTarget : public Object {
public: 
	virtual Object * clone() const { return new BallisticMissileTarget(*this); }
	
	BallisticMissileTarget() : Object("mark"), _reaction(true) {
		set_directions_number(1);
	}
	
	void on_spawn() { 
		GET_CONFIG_VALUE("objects.target.reaction-time", float, rt, 0.2f);
		mrt::randomize(rt, rt / 10);
		_reaction.set(rt);

		play("main", true);
	}
	void calculate(const float dt) {
		if (!_reaction.tick(dt))
			return;
		v2<float> pos, vel;
		if (get_nearest(ai::Targets->troops, speed * 5.0f, pos, vel, false)) {
			_velocity = pos;
			return;
		}
	}

	virtual void serialize(mrt::Serializator &s) const {
		s.add(_reaction);
		Object::serialize(s);
	}

	virtual void deserialize(const mrt::Serializator &s) {
		s.get(_reaction);
		Object::deserialize(s);
	}
private: 
	Alarm _reaction;
};

REGISTER_OBJECT("ballistic-missile", BallisticMissile, ());
REGISTER_OBJECT("ballistic-missile-target", BallisticMissileTarget, ());
