#ifndef BTANKS_AI_HERD_H__
#define BTANKS_AI_HERD_H__

#include "math/v3.h"

class Object;

namespace ai {

class Herd {
public:
	virtual ~Herd() {}
	void calculateV(v3<float> &velocity, Object *sheep, const int leader, const float distance);

private:
	virtual const int getComfortDistance(const Object *other) const = 0;
};

}

#endif
