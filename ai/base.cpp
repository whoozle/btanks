#include "base.h"
#include <assert.h>

using namespace ai;

Base::Base() : Object("player") {}

void Base::onSpawn() {
	//initialize program
	_weapon1[1] = "nx";
	
	_weapon_name['1'] = "missiles:guided";
	_weapon_name['2'] = "missiles:smoke";
	_weapon_name['3'] = "missiles:dumb";
	_weapon_name['4'] = "missiles:nuke";
	_weapon_name['5'] = "missiles:boomerang";
	_weapon_name['6'] = "missiles:stun";
	_weapon_name['7'] = "mines:regular";
	
	_effect_name['1'] = "invulnerability";
	_effect_name['2'] = "stunned";
	_effect_name['3'] = "machinegunner";
	
	_movement[1] = "pw1:x";
	_movement[2] = "Pw1:w1";
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

const bool Base::getCounter(int &n, const char c, const char t) {	
	switch(c) {
	case 'h': 
		n = hp;
		return true;

	case 'w': {
		std::string mod;
		switch(t) {
			case '1' : mod = "mod"; break;
			case '2' : mod = "alt-mod"; break;
			default: throw_ex(("weapon mod '%c' is invalid", t));
		}
		
		if (!has(mod))
			return false;
		
		n = get(mod)->getCount();
		return true;
	}
	
	case 'e': {
		const std::string name = _effect_name[t];
		if (name.empty())
			throw_ex(("unknown effect '%c' requested", t));
		if (!isEffectActive(name))
			return false;
		n = (int) (getEffectTimer(name) * 1000);
		return true;
	}
	default: 
		throw_ex(("getCounter('%c') is invalid", c));
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
				const bool far = c == 'F' || c == 'f';

				Point p;
				if (!getPoint(p, o))
					return false;
					
				//LOG_DEBUG(("found object '%c' at %g %g", o, p.x, p.y));
				
				float distance = _traits.get("near", o, 100, 300);
				if (far) { 
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
		
		case 'L':
//		case 'M':
		case 'P':
			neg = true;
		case 'l':
//		case 'm':
		case 'p':
			{
				const char o = program[i++];
				const char t = (o == 'h')?'#':program[i++];;
				int n;

				bool present = getCounter(n, o, t);
				if (!present && (c == 'p' || c == 'P'))
					return neg;
				
				float amount = _traits.get("less", o, 0, 10);
				bool r = n <= amount;
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


