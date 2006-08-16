#ifndef __BTANKS_BASE_AI_H__
#define __BTANKS_BASE_AI_H__

#include "math/matrix.h"
#include "player.h"
#include "math/v3.h"
#include <deque>

class BaseAI : public Player {
public: 
	BaseAI(const bool stateless);
	BaseAI(const std::string &animation, const bool stateless);
	typedef v3<int> WayPoint;
	typedef std::deque<WayPoint> Way;
	const bool getPath(Way &way, const v3<float> &pos);
};

#endif

