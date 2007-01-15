#ifndef BTANKS_BASE_AI_H__
#define BTANKS_BASE_AI_H__

#include "object.h"
#include "traits.h"
#include <string>
#include <set>
#include "alarm.h"

namespace ai {
class Base : public virtual Object {
public: 
	Base();
	virtual void calculate(const float dt);
	virtual void onSpawn();
	
protected: 
	void addEnemyClass(const std::string &classname);
	void addBonusName(const std::string &rname);
private: 
	Alarm _reaction_time;
	ai::Traits _traits;
	std::set<std::string> _enemies, _bonuses;
	int _target_id;
	v3<int> _target_position;
};
}

#endif

