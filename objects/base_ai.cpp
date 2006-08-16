#include "base_ai.h"
#include "world.h"
#include "tmx/map.h"

BaseAI::BaseAI(const bool stateless) : Player(stateless) {}
BaseAI::BaseAI(const std::string &animation, const bool stateless) : Player(animation, stateless) {}


void BaseAI::getPath(Matrix<int> &path, const v3<float> dpos) {
	World->getImpassabilityMatrix(path);
	v3<float> pos;
	convertToAbsolute(pos, dpos);
	//LOG_DEBUG(("getPath"));
	pos /= IMap::pathfinding_step;
}
