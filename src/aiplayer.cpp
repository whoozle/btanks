#include "aiplayer.h"
#include "world.h"

#include "mrt/logger.h"

AIPlayer::AIPlayer(const std::string &animation) : Player("ai", animation, true) {}

void AIPlayer::tick(const float dt) {	
	v3<float> pos, vel;
	if (getNearest("human", pos, vel)) {
		//LOG_DEBUG(("found human: %f %f", pos.x, pos.y));
		v3<float> my_pos;
		getPosition(my_pos);
		_velocity = pos - my_pos;
		
		if (_velocity.lenght() < 100) {
			_velocity.clear();
			return;
		}
		
		static float threshold = 12;
		
		if (_velocity.x >= -threshold && _velocity.x <= threshold) _velocity.x = 0;
		if (_velocity.y >= -threshold && _velocity.y <= threshold) _velocity.y = 0;
		//LOG_DEBUG(("v: %f %f", _velocity.x, _velocity.y));
	} else _velocity.clear();
	Player::tick(dt);
}
