#ifndef __BTANKS_BASE_AI_H__
#define __BTANKS_BASE_AI_H__

#include "math/matrix.h"
#include "player.h"
#include "math/v3.h"

class BaseAI : public Player {
public: 
	BaseAI(const bool stateless);
	BaseAI(const std::string &animation, const bool stateless);

	void getPath(Matrix<int> &path, const v3<float> pos);
};

#endif

