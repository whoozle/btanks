#include "object.h"
#include "registrar.h"
#include "mrt/random.h"
#include "config.h"
#include "world.h"
#include "math/unary.h"

class BallisticMissile : public Object {
public: 
	virtual Object * clone() const { return new BallisticMissile(*this); }
	
	BallisticMissile() : Object("ballistic-missile"), _fall(false), _reaction(true)	{
		setDirectionsNumber(16);
		piercing = true;
	}
	
	void onSpawn() {
		play("main", true);
		float duration = 5.0f;
		_fall.set(duration);
		float reaction = 0.05f;
		mrt::randomize(reaction, reaction / 10);
		_reaction.set(reaction);
		
		setDirection(4);
		_direction = _velocity = v2<float>(0, -1);

		Object *target = spawn("ballistic-missile-target", "target");
		target_id = target->getID();
		speed_backup = speed;
	}
	
	void calculate(const float dt) {
		bool react = _reaction.tick(dt), falling = _fall.tick(dt);
		if (!falling) {
			v2<float> pos = getPosition();
			if (react && pos.y < 0) {
				Object *target = World->getObjectByID(target_id);
				if (target == NULL) {
					Object::emit("death", NULL); //just hide 
					return;
				}
				speed = target->speed * 1.5f;
				_velocity = getRelativePosition(target);
				_velocity.y = -1;
				//LOG_DEBUG(("correcting: %g", _velocity.x));
			}
		} else if (react) { //falling and react
			speed = speed_backup;
			setDirection(12);
			Object *target = World->getObjectByID(target_id);
			
			if (target == NULL) {
				Object::emit("death", NULL); //just hide 
				return;
			}
			v2<float> pos = getPosition() + size, tpos = target->getCenterPosition();
			_velocity = tpos - pos;
			if (_velocity.y <= 0) {
				if (animation == "nuke-missile") {
					spawn("nuke-explosion", "nuke-explosion");
				}
			
				emit("death", NULL);
				target->emit("death", NULL);
			} else {
				if (math::abs(_velocity.x) * 5 > _velocity.y)
					_velocity.x = _velocity.y / 5;
			}
		}
	}

	virtual void serialize(mrt::Serializator &s) const {
		Object::serialize(s);
		s.add(_fall);
		s.add(_reaction);
		s.add(target_id);
		s.add(speed_backup);
	}

	virtual void deserialize(const mrt::Serializator &s) {
		Object::deserialize(s);
		s.get(_fall);
		s.get(_reaction);
		s.get(target_id);
		s.get(speed_backup);
	}
private: 
	Alarm _fall, _reaction;
	float speed_backup;
	int target_id;
};

class BallisticMissileTarget : public Object {
public: 
	virtual Object * clone() const { return new BallisticMissileTarget(*this); }
	
	BallisticMissileTarget() : Object("mark"), _reaction(true) {
		setDirectionsNumber(1);
		if (_targets.empty()) {
			_targets.insert("fighting-vehicle");
			_targets.insert("trooper");
			_targets.insert("kamikaze");
			_targets.insert("boat");
			_targets.insert("helicopter");
			_targets.insert("monster");
			_targets.insert("watchtower");
		}
	}
	
	void onSpawn() { 
		GET_CONFIG_VALUE("objects.target.reaction-time", float, rt, 0.2f);
		mrt::randomize(rt, rt / 10);
		_reaction.set(rt);

		play("main", true);
	}
	void calculate(const float dt) {
		if (!_reaction.tick(dt))
			return;
		v2<float> pos, vel;
		if (getNearest(_targets, speed * 5.0f, pos, vel, false)) {
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
	
	static std::set<std::string> _targets;
};

std::set<std::string> BallisticMissileTarget::_targets;

REGISTER_OBJECT("ballistic-missile", BallisticMissile, ());
REGISTER_OBJECT("ballistic-missile-target", BallisticMissileTarget, ());
