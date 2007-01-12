#ifndef BTANKS_BASE_AI_H__
#define BTANKS_BASE_AI_H__

#include "object.h"
#include "traits.h"

namespace ai {
class Base : public virtual Object {
public: 
	Base();
	virtual void calculate(const float dt);
	virtual void onSpawn();
	
protected: 
	//
private: 
	//predicates
	typedef v3<float> Point;
	
	//end of preds
	const bool getPoint(Point &p, const char c);
	
	//evaluating stuff
	typedef std::map<const float, std::string> RuleMap;

	const bool eval(const RuleMap &rules);
	const bool eval(const std::string &program);
	

	ai::Traits _traits;
	
	RuleMap _weapon1, _weapon2, _movement;
};
}

#endif

