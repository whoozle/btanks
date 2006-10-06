#ifndef __WORLD_BASE_OBJECT_H__
#define __WORLD_BASE_OBJECT_H__
/* Battle Tanks Game
 * Copyright (C) 2006 Battle Tanks team
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
#include <string>
#include "math/v3.h"
#include "mrt/serializable.h" 
#include "player_state.h"
#include "object_common.h"

namespace sdlx {
	class Surface;
}

class BaseObject : public mrt::Serializable {
public:
	v3<float> size;
	float mass, speed, ttl, impassability;
	int hp, max_hp;
	
	bool piercing, pierceable;
	
	std::string classname;
	
	BaseObject(const std::string &classname);
	virtual ~BaseObject();
	
	virtual void tick(const float dt) = 0;
	virtual void render(sdlx::Surface &surf, const int x, const int y) = 0;
	virtual void emit(const std::string &event, BaseObject * emitter = NULL);
	
	const float getCollisionTime(const v3<float> &pos, const v3<float> &vel, const float r) const;
	
	inline const bool isDead() const { return _dead; }
	inline const int getID() const { return _id; }

	virtual void serialize(mrt::Serializator &s) const;
	virtual void deserialize(const mrt::Serializator &s);
	
	const std::string dump() const;
	void inheritParameters(const BaseObject *other);
	void convertToAbsolute(v3<float> &pos, const v3<float> &dpos);

	inline PlayerState & getPlayerState() { return _state; }
	const bool updatePlayerState(const PlayerState &state);

	void follow(const BaseObject *obj, const GroupType mode = Fixed);
	void follow(const int id); //add mode
	inline const int getLeader() const { return _follow; }

	void heal(const int hp);
	virtual const bool take(const BaseObject *obj, const std::string &type);
	virtual void addDamage(BaseObject *from, const int hp, const bool emitDeath = true);
	void addDamage(BaseObject *from, const bool emitDeath = true);
	
	const v3<float> getRelativePos(const BaseObject *obj) const;
	inline void getPosition(v3<float> &position) { position = _position; }
	inline void getPosition(v3<int> &position) { position = _position.convert<int>(); }

	inline void getCenterPosition(v3<float> &position) { position = _position; position += size / 2; }
	inline void getCenterPosition(v3<int> &position) { position = (_position + size / 2).convert<int>();  }

	void getInfo(v3<float> &pos, v3<float> &vel) const;
	void updateStateFromVelocity();
	void setZ(const float z); 

	void getTargetPosition8(v3<float> &position, const v3<float> &target, const std::string &weapon);
	
protected:
	int _id;
	int _follow;
	v3<float> _follow_position;
	PlayerState _state;
	v3<float> _velocity, _direction, _velocity_fadeout;
	float _moving_time, _idle_time;
	
	inline const v3<float> & getPosition() { return _position; }

	virtual void calculate(const float dt);

	void disown();
	bool need_sync;

private:

	bool _dead;
	v3<float> _position;
	int _owner_id;
	
	friend class IWorld;
};

#endif

