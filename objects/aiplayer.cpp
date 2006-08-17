#include "aiplayer.h"
#include "tmx/map.h"
#include "sdlx/rect.h"
#include "mrt/logger.h"
#include "resource_manager.h"

REGISTER_OBJECT("ai-player", AIPlayer, ());

#define REACTION_TIME (0.100)
#define PATHFINDING_TIME (0.200)

AIPlayer::AIPlayer() : BaseAI(true), 
	_reaction_time(REACTION_TIME, true), _refresh_waypoints(PATHFINDING_TIME, true) {}

AIPlayer::AIPlayer(const std::string &animation) : BaseAI(animation, true), 
	_reaction_time(REACTION_TIME, true), _refresh_waypoints(PATHFINDING_TIME, true)  {}

void AIPlayer::tick(const float dt) {	
	//LOG_DEBUG(("dt = %f", dt));
	if (!_reaction_time.tick(dt)) {
		Player::tick(dt);
		return;
	}
	
	v3<float> bpos, pos, vel;
	bool found_bullet = false;

	if (getNearest("bullet", bpos, vel)) {
		//LOG_DEBUG(("AAA!!!"));
		float t = getCollisionTime(bpos, vel, (size.x + size.y)/2);
		//LOG_DEBUG(("collision time: %f", t));
		if (t >= 0) {
			_velocity.x = -vel.y;
			_velocity.y = vel.x;
			found_bullet = true;
		}
	} 

	Way way;
	const bool refresh_path = _refresh_waypoints.tick(dt);
	
	if (getNearest("player", pos, vel, (refresh_path || !isDriven())?&way:0)) {
		//LOG_DEBUG(("found human: %f %f", pos.x, pos.y));
		const bool player_close = pos.quick_length() < IMap::pathfinding_step * IMap::pathfinding_step * 9; //3xpathfinding step
		
		if (found_bullet && bpos.quick_length() < pos.quick_length()) {
			//LOG_DEBUG(("bpos: %g, player: %g", bpos.quick_length(), pos.quick_length()));
			goto skip_player;
		}
		
		if (player_close) {
			if (isDriven())
				LOG_DEBUG(("player is too close, turning off waypoints..."));
			way.clear();
			setWay(way);
		}
		
		if (!way.empty()) {
			//LOG_DEBUG(("finding path..."));
			way.pop_back();
			setWay(way);
		} else {	
			if (!isDriven()) {
				LOG_WARN(("no path."));
				_velocity = pos; //straight to player.
			}
		} 

		static float threshold = 12;
		
		if (pos.x >= -threshold && pos.x <= threshold) { 
			pos.x = 0;
			_state.fire = true;
		}
		if (pos.y >= -threshold && pos.y <= threshold) {
			pos.y = 0;
			_state.fire = true;
		}

		float tg = pos.x != 0 ?(pos.y / pos.x - 1):100;
		if (tg < 0) tg = -tg;
		
		//LOG_DEBUG(("tg = %f", tg));
		if (tg > 0.577350269189625798 && tg < 1.7320508075688778) {
			_state.fire = true;
		}
		
		if (pos.length() < 3*IMap::pathfinding_step / 2) {
			_velocity.clear();
			Player::tick(dt);
			return;
		}

		//LOG_DEBUG(("v: %f %f", pos.x, pos.y));
	  } else {
	  	_velocity.clear();
	  }
	
	skip_player:
	
	Player::tick(dt);
}

Object * AIPlayer::clone(const std::string &opt) const {
	AIPlayer *p = NULL;
	TRY { 
		//LOG_DEBUG(("cloning player with animation '%s'", opt.c_str()));
		p = new AIPlayer(*this);
		p->setup(opt);
	} CATCH("clone", { delete p; throw; });
	return p;
}
