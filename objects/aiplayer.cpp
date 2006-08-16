#include "aiplayer.h"
#include "tmx/map.h"
#include "sdlx/rect.h"
#include "mrt/logger.h"
#include "resource_manager.h"

REGISTER_OBJECT("ai-player", AIPlayer, ());

#define REACTION_TIME (0.100)

AIPlayer::AIPlayer() : BaseAI(true), _reaction_time(REACTION_TIME, true) {}

AIPlayer::AIPlayer(const std::string &animation) : BaseAI(animation, true), _reaction_time(REACTION_TIME, true) {}

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
	if (getNearest("player", pos, vel, &way)) {
		//LOG_DEBUG(("found human: %f %f", pos.x, pos.y));
		
		if (found_bullet && bpos.quick_length() < pos.quick_length()) {
			//LOG_DEBUG(("bpos: %g, player: %g", bpos.quick_length(), pos.quick_length()));
			goto skip_player;
		}
		
		if (!way.empty()) {
			//LOG_DEBUG(("path finding turned on"));
			v3<float> me;
			getPosition(me);
/*			const WayPoint &wp = *way.begin();
			LOG_DEBUG(("way point: %g %g --> %d %d (%d:%d)", me.x, me.y, wp.x, wp.y, wp.y/IMap::pathfinding_step, wp.x/IMap::pathfinding_step));
			_velocity.x = wp.x - me.x;
			_velocity.y = wp.y - me.y;
			goto skip_player;
*/
			LOG_DEBUG(("%d waypoints", way.size()));
			sdlx::Rect mr(me.x, me.y, size.x, size.y);

			Way::reverse_iterator dst_it = way.rend();
			for(Way::reverse_iterator i = way.rbegin(); i != way.rend(); ++i) {
				LOG_DEBUG(("%d %d %d %d", i->x, i->y, (int)me.x, (int)me.y));
				sdlx::Rect wr(i->x, i->y, IMap::pathfinding_step, IMap::pathfinding_step);
				if (wr.in(me.x, me.y)) {
					break;
				}
				dst_it = i;
			}
			assert (dst_it != way.rend());
			const WayPoint &wp = *dst_it;
			
			LOG_DEBUG(("got waypoint: %d %d", wp.x, wp.y));
			_velocity.x = wp.x - me.x;
			_velocity.y = wp.y - me.y;
			_velocity.normalize();
			LOG_DEBUG(("vel : %g %g", _velocity.x, _velocity.y));
			goto skip_player;
		} else {	
			_velocity.clear();
			LOG_WARN(("no path."));
			//throw_ex(("byebye"));
			goto skip_player;
		} 

		_velocity = pos;
		
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
		
		if (_velocity.lenght() < 100) {
			_velocity.clear();
			Player::tick(dt);
			return;
		}
		//LOG_DEBUG(("v: %f %f", _velocity.x, _velocity.y));
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
