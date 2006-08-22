#ifndef __BTANKS_AIPLAYER_H__
#define __BTANKS_AIPLAYER_H__

#include "alarm.h"
#include "tank.h"

class AIPlayer : public Tank {
public: 
	AIPlayer();
	AIPlayer(const std::string &animation);

	virtual void tick(const float dt);
	virtual Object * clone(const std::string &opt) const;
private: 
	Alarm _reaction_time, _refresh_waypoints;
};

#endif
