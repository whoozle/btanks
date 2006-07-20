#include "aiplayer.h"
#include "world.h"

AIPlayer::AIPlayer(const std::string &animation) : Player(animation, true) {}

void AIPlayer::tick(const float dt) {
	Player::tick(dt);
}
