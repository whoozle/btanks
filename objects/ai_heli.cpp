#include "heli.h"
#include "config.h"
#include "resource_manager.h"
#include "player_manager.h"
#include "tmx/map.h"
#include "mrt/random.h"

class AIHeli : public Heli {
public:
	AIHeli() : Heli("helicopter"), _reaction(true) {}
	virtual void onSpawn();
	void calculate(const float dt);
	virtual void serialize(mrt::Serializator &s) const {
		Heli::serialize(s);
		_reaction.serialize(s);
	}
	virtual void deserialize(const mrt::Serializator &s) {
		Heli::deserialize(s);
		_reaction.deserialize(s);
	}

	virtual Object * clone() const { return new AIHeli(*this); }
	
private: 
	Alarm _reaction;	
};

void AIHeli::onSpawn() {
	GET_CONFIG_VALUE("objects.helicopter.reaction-time", float, rt, 0.1);
	mrt::randomize(rt, rt/10);
	_reaction.set(rt);
	Heli::onSpawn();
}

void AIHeli::calculate(const float dt) {
	if (!_reaction.tick(dt))
		goto done;
		
	_state.fire = true;
	
	GET_CONFIG_VALUE("engine.mass-acceleration-divisor", float, ac_div, 1000.0);

	{
		const float ac_t = mass / ac_div * 0.8;
		_state.alt_fire = _moving_time >= ac_t;
	}
	
	if (!isDriven() && !PlayerManager->isClient()) { 
		Way way;
		v2<int> size = Map->getSize();
		
		for(int i = 0; i < 3; ++i) {
			v2<int> next_target;
			next_target.x = mrt::random(size.x);
			next_target.y = mrt::random(size.y);
			way.push_back(next_target);		
		}
		setWay(way);
	}

done: 	
	calculateWayVelocity();
	updateStateFromVelocity();

	GET_CONFIG_VALUE("objects.helicopter.rotation-time", float, rt, 0.2);
	limitRotation(dt, rt, true, true);	
}

REGISTER_OBJECT("helicopter", AIHeli, ());
