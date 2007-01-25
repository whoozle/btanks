#ifndef __WORLD_BASE_OBJECT_H__
#define __WORLD_BASE_OBJECT_H__

/* Battle Tanks Game
 * Copyright (C) 2006-2007 Battle Tanks team
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
#include <deque>

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
	
	const v3<float> getRelativePosition(const BaseObject *obj) const;

	inline const v3<float> & getPosition() const { return _position; }
	inline void getPosition(v3<float> &position) const { position = _position; }
	inline void getPosition(v3<int> &position) const { position = _position.convert<int>(); }

	inline void getCenterPosition(v3<float> &position) const { position = _position; position += size / 2; }
	inline void getCenterPosition(v3<int> &position) const { position = (_position + size / 2).convert<int>();  }

	void getInfo(v3<float> &pos, v3<float> &vel) const;
	void updateStateFromVelocity();
	void setZ(const float z); 
	inline const float getZ() const { return _position.z; }
	
	void disown();

	void addOwner(const int oid);
	void prependOwner(const int oid); //to avoid truncation of this owner. 
	const int _getOwner() const;
	const bool hasOwner(const int oid) const;
	const bool hasSameOwner(const BaseObject *other) const;
	void removeOwner(const int oid);
	void truncateOwners(const int n);
	inline void getOwners(std::deque<int> &owners) const { owners = _owners; }
	
	const int getSummoner() const { return _spawned_by; }
	
protected:
	int _id;
	int _follow;
	v3<float> _follow_position;
	PlayerState _state;
	v3<float> _velocity, _direction, _velocity_fadeout;
	float _moving_time, _idle_time;
	

	virtual void calculate(const float dt) = 0;

	bool need_sync, _dead;

private:

	v3<float> _position;
	std::deque<int> _owners;
	int _spawned_by;
	
	friend class IWorld;
};

#endif

