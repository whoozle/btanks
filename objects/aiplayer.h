#ifndef __BTANKS_AIPLAYER_H__
#define __BTANKS_AIPLAYER_H__

#include "player.h"

class AIPlayer : public Player {
public: 
	AIPlayer();
	AIPlayer(const std::string &animation);

	virtual void tick(const float dt);
};

#endif
