
/* Battle Tanks Game
 * Copyright (C) 2006-2008 Battle Tanks team
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */
#include "heli.h"
#include "config.h"
#include "registrar.h"
#include "player_manager.h"
#include "player_slot.h"

class RaiderHeli : public Heli {
public:
	virtual Object * clone() const { return new RaiderHeli(*this); }
	RaiderHeli() : Heli("helicopter"), _player(-1), _leaving(false), _toggle(true) {}
	
	virtual void onSpawn();
	void calculate(const float dt);
	virtual void serialize(mrt::Serializator &s) const {
		Heli::serialize(s);
		s.add(_player);
		s.add(_leaving);
		s.add(_toggle);
	}
	virtual void deserialize(const mrt::Serializator &s) {
		Heli::deserialize(s);
		s.get(_player);
		s.get(_leaving);
		s.get(_toggle);
	}

private: 
	int _player;
	Alarm _leaving, _toggle;
};


void RaiderHeli::onSpawn() {
	Heli::onSpawn();
	_player = -1;
/*
*/
}

#include "tmx/map.h"

void RaiderHeli::calculate(const float dt) {
	if (_player == -1) {
	//deferred initialization
		int players = 0;
		int i, n = PlayerManager->getSlotsCount();
		for(i = 0; i < n; ++i) {
			const PlayerSlot &slot = PlayerManager->getSlot(i);
			if (slot.empty())
				continue;
			++players;
		}

		if (players == 0) {
			LOG_DEBUG(("no players... "));
			emit("death", NULL);
			return;
		}
		LOG_DEBUG(("setting up %d players", players));
		float attack = 20.0f;
		//Config->get("objects." + registered_name + ".attack-duration", attack, 30.0f);
		_toggle.set(attack);
		_leaving.set(attack * players * 2);
		_player = 0;
	}

	if (!_variants.has("no-escape") && _leaving.tick(dt)) {
		v2<int> pos, map_size = Map->getSize();
		getPosition(pos);
		pos += (size * 0.9).convert<int>();
		if (pos.x > map_size.x || pos.y > map_size.y) {
			LOG_DEBUG(("escaped"));
			Object::emit("death", NULL);
		}
		_velocity = v2<float>(4, 3);
		
		goto done;
	}

	{	
	//main ai
		PlayerSlot &slot = PlayerManager->getSlot(_player);
		Object *player = slot.getObject();
		if (player == NULL || _toggle.tick(dt)) {
			(++_player) %= PlayerManager->getSlotsCount();
			//LOG_DEBUG(("changing player to %d", _player));
			return;
		}
		//LOG_DEBUG(("attacking player %d", _player));
		
		v2<float> pos, vel;
		pos = getRelativePosition(player);
		player->get_velocity(vel);
		vel.normalize();
		
		float est = pos.length() / speed;
		
		v2<float> dir = pos  + speed * vel * est - v2<float>(0, 128); //bomb correction
		if (dir.length() > 64) {
			dir.normalize();
			_velocity.normalize();
			_velocity = _velocity * 2 + dir * 3;
			_velocity.normalize();
		} else {
			_velocity = _direction;
			//LOG_DEBUG(("idle velocity %g %g", _velocity.x, _velocity.y));
		}
	}

done: 
	//common part
	GET_CONFIG_VALUE("engine.mass-acceleration-divisor", float, ac_div, 1000.0f);

	const float ac_t = mass / ac_div * 0.8;
	_state.alt_fire = _moving_time >= ac_t;

	calculateWayVelocity();

	GET_CONFIG_VALUE("objects.helicopter.rotation-time", float, rt, 0.2f);
	limitRotation(dt, rt, false, false);	
	updateStateFromVelocity();
}

REGISTER_OBJECT("raider-helicopter", RaiderHeli, ());
