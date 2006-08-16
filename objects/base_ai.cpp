#include "base_ai.h"
#include "world.h"
#include "tmx/map.h"
#include <deque>

BaseAI::BaseAI(const bool stateless) : Player(stateless) {}
BaseAI::BaseAI(const std::string &animation, const bool stateless) : Player(animation, stateless) {}
