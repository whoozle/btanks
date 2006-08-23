#ifndef __BTANKS_AIPLAYER_H__
#define __BTANKS_AIPLAYER_H__

#include "alarm.h"
#include "tank.h"

class AITank : public Tank {
public: 
	AITank();
	AITank(const std::string &animation);

	virtual void calculate(const float dt);
	virtual Object * clone(const std::string &opt) const;
private: 
	Alarm _reaction_time, _refresh_waypoints;
};

#endif
