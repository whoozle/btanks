#ifndef __WORLD_BASE_OBJECT_H__
#define __WORLD_BASE_OBJECT_H__

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
#include <string>
#include <deque>
#include <set>

#include "export_btanks.h"
#include "math/v2.h"
#include "mrt/serializable.h" 
#include "player_state.h"
#include "object_common.h"
#include "variants.h"

namespace sdlx {
	class Surface;
}

namespace ai {
	class Buratino;
	class Waypoints;
}

class BTANKSAPI BaseObject : public mrt::Serializable {
public:
	v2<float> size;
	float mass, speed, ttl, impassability;
	int hp, max_hp;
	
	bool piercing, pierceable;
	
	std::string classname;
	
	bool disable_ai;
	
	BaseObject(const std::string &classname);
	virtual ~BaseObject();
	
	void update_variants(const Variants &vars, const bool remove_old = false);
	
	virtual void tick(const float dt) = 0;
	virtual void render(sdlx::Surface &surf, const int x, const int y) = 0;
	void get_velocity(v2<float> &vel) const { vel = _velocity; vel.normalize(); vel *= speed; }
	static const float get_collision_time(const v2<float> &pos, const v2<float> &vel, const float r);
	
	inline const bool is_dead() const { return _dead; }
	inline const int get_id() const { return _id; }

	virtual void serialize(mrt::Serializator &s) const;
	virtual void deserialize(const mrt::Serializator &s);
	
	const std::string dump() const;
	void inherit_parameters(const BaseObject *other);

	inline const PlayerState & get_player_state() const { return _state; }
	const bool update_player_state(const PlayerState &state);

	void heal(const int hp);
	virtual const bool take(const BaseObject *obj, const std::string &type);
	
	void update_state_from_velocity();
	void set_z(const int z, const bool absolute = false); 
	inline const int get_z() const { return _z; }
	
	void disown();
	inline const bool has_owners() const { return !_owner_set.empty(); }
	void add_owner(const int oid);
	void prepend_owner(const int oid); //to avoid truncation of this owner. 
	const bool has_owner(const int oid) const;
	const bool has_same_owner(const BaseObject *other, const bool skip_cooperative = false) const;
	void remove_owner(const int oid);
	void truncate_owners(const int n);
	inline void get_owners(std::deque<int> &owners) const { owners = _owners; }
	void copy_owners(const BaseObject *from);
	void copy_special_owners(const BaseObject *from);
	
	inline const int get_summoner() const { return _spawned_by; }
	
	void interpolate();
	void uninterpolate();
	
	virtual void get_impassability_penalty(const float impassability, float &base, float &base_value, float &penalty) const;

	const float get_effective_impassability(const float impassability) const;
	
	const Variants & get_variants() const { return _variants; }
	
	inline const v2<float>& get_direction_vector() const { return _direction; }
	
protected:
	int _id;
	PlayerState _state;
	v2<float> _velocity, _direction;
	
	virtual void calculate(const float dt) = 0;

	bool _need_sync, _dead;
	Variants _variants;
	v2<float> _position;

private:
	
	//do not serialize interpolation stuff.
	v2<float> _interpolation_vector, _interpolation_position_backup;
	float _interpolation_progress;
	
	int _z;

	std::deque<int> _owners;
	std::set<int> _owner_set;

	int _spawned_by;
	
	friend class IWorld;
	friend class Object;
	friend class Teleport;
	friend class ai::Buratino;
	friend class ai::Waypoints;
};

#endif

