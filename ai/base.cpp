#include "base.h"
#include <assert.h>

using namespace ai;

Base::Base() : Object("player") {}

void Base::onSpawn() {
	//initialize program
	_weapon1[1] = "nx";
}

const bool Base::eval(const RuleMap &rules) {
	for(RuleMap::const_iterator i = rules.begin(); i != rules.end(); ++i) {
		if (eval(i->second))
			return true;
	}
	return false;
}

const bool Base::getPoint(Point &position, const char c) {
	static std::vector<std::string> enemy_classes;
	if (enemy_classes.empty()) {
		enemy_classes.push_back("player");
		enemy_classes.push_back("kamikaze");
		enemy_classes.push_back("trooper");
	}
	switch(c) {
	case 'x': {
		Point v;
		return getNearest(enemy_classes, position, v);
	}
	default: 
		throw_ex(("getPoint('%c') is invalid", c));
	}
	assert(0);
	return false;
}

const bool Base::eval(const std::string &program) {
	for(size_t i = 0; i < program.size(); ) {
		const char c = program[i++];
		bool neg = false;

		switch(c) {

		case 'N':
		case 'F':
			neg = true;
		case 'n': 
		case 'f':
			
			{
				const char o = program[i++];
				Point p;
				if (!getPoint(p, o))
					return false;
					
				//LOG_DEBUG(("found object '%c' at %g %g", o, p.x, p.y));
				
				float distance = _traits.get("near", o, 100, 300);
				if (c == 'F' || c == 'f') { 
					distance += _traits.get("far", o, 300, 1000);
					neg = !neg;
				}

				bool r = p.length() <= distance;
				if (neg) 
					r = !r;

				if (!r)
					return false;
			}			
			break;
		default: 
			throw_ex(("invalid condition '%c' in position %u", c, i));
		}
	}
	return true;
}


void Base::calculate(const float dt) {
	_state.fire = eval(_weapon1);
	_state.alt_fire = eval(_weapon2);
	
	eval(_movement);
	
	//calculate pathfinding here.
	if (!calculatingPath() && !isDriven()) {
		//idle.
		_velocity.clear();
	}
	
	Way way;
	if (calculatingPath() && findPathDone(way)) {
		setWay(way);
	}
	calculateWayVelocity();
}


