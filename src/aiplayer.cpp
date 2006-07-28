#include "aiplayer.h"
#include "world.h"

#include "mrt/logger.h"

AIPlayer::AIPlayer(const std::string &animation) : Player(animation, true) {}

void AIPlayer::tick(const float dt) {	
	//LOG_DEBUG(("dt = %f", dt));
	v3<float> pos, vel;
	bool skip_human = false;

	if (getNearest("bullet", pos, vel)) {
		//LOG_DEBUG(("AAA!!!"));
		float t = getCollisionTime(pos, vel);
		if (t >= 0) {
			//LOG_DEBUG(("collision time: %f", t));
			_velocity.x = -vel.y;
			_velocity.y = vel.x;
			skip_human = true;
		}
	} 
	
	if (!skip_human) {
	  if (getNearest("player", pos, vel)) {
		//LOG_DEBUG(("found human: %f %f", pos.x, pos.y));
		v3<float> my_pos;
		getPosition(my_pos);
		_velocity = pos - my_pos;
		
		if (_velocity.lenght() < 100) {
			_velocity.clear();
			return;
		}
		
		static float threshold = 12;
		
		if (_velocity.x >= -threshold && _velocity.x <= threshold) { 
			_velocity.x = 0;
			_state.fire = true;
		}
		if (_velocity.y >= -threshold && _velocity.y <= threshold) {
			_velocity.y = 0;
			_state.fire = true;
		}
		float tg = _velocity.x != 0 ?(_velocity.y / _velocity.x - 1):100;
		if (tg < 0) tg = -tg;
		
		//LOG_DEBUG(("tg = %f", tg));
		if (tg > 0.577350269189625798 && tg < 1.7320508075688778) {
			_state.fire = true;
		}
		
		//LOG_DEBUG(("v: %f %f", _velocity.x, _velocity.y));
	  } else {
	  	_velocity.clear();
	  }
	}

	Player::tick(dt);
}
