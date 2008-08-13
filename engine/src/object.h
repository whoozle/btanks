#ifndef __BTANKS_ANIMATED_OBJECT__
#define __BTANKS_ANIMATED_OBJECT__

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

/* 
 * Additional rights can be granted beyond the GNU General Public License 
 * on the terms provided in the Exception. If you modify this file, 
 * you may extend this exception to your version of the file, 
 * but you are not obligated to do so. If you do not wish to provide this
 * exception without modification, you must delete this exception statement
 * from your version and license this file solely under the GPL without exception. 
*/

#include <string>
#include <map>
#include <set>
#include <queue>

#include "export_btanks.h"
#include "base_object.h"
#include "alarm.h"
#include "math/v2.h"
#include "math/matrix.h"
#include "object_common.h"

namespace sdlx {
	class Rect;
	class Surface;
	class CollisionMap;
}

class AnimationModel;
class Animation;
class Pose;

class BTANKSAPI Object : public BaseObject {
public:
	const v2<float> get_relative_position(const Object *obj) const;
	inline const v2<float> get_position() const { return _parent == NULL? _position: _position + _parent->get_position(); }

	template<typename T>
	inline void get_position(v2<T> &position) const { 
		position = _position.convert<T>(); 
		if (_parent != NULL) {
			v2<T> ppos;
			_parent->get_position(ppos);
			position += ppos;
		}
	}

	inline const v2<float> get_center_position() const { return get_position() + size / 2; }
	template<typename T>
	inline void get_center_position(v2<T> &position) const { get_position<T>(position); position += (size / 2).convert<T>(); }

	const std::string registered_name; 
	
	std::string animation;
	float fadeout_time;

	Object(const std::string &classname); //do not use parent - internal
	~Object();
	
	void init(const std::string &animation); //do not use it, needed for resman

	virtual Object * clone() const;
	Object * deep_clone() const;
	
	bool playing_sound(const std::string &name) const;
	void play_sound(const std::string &name, const bool loop, const float gain = 1.0);
	void play_random_sound(const std::string &classname, const bool loop, const float gain = 1.0);
	void fadeout_sound(const std::string &name);

	inline const int get_direction() const { return _direction_idx; }
	inline const int get_directions_number() const { return _directions_n; }
	virtual void set_direction(const int dir);
	void set_directions_number(const int dirs);
	
	virtual void tick(const float dt);
	virtual void render(sdlx::Surface &surface, const int x, const int y);
	const bool collides(const Object *other, const int x, const int y, const bool hidden_by_other = false) const;
	const bool collides(const sdlx::CollisionMap *other, const int x, const int y, const bool hidden_by_other = false) const;

	// animation:
	void play(const std::string &id, const bool repeat = false);
	void play_now(const std::string &id);
	void cancel();
	void cancel_repeatable();
	void cancel_all();
	inline const std::string& get_state() const {
		static const std::string empty;
		if (_events.empty())
			return empty;
		return _events.front().name;
	}
	const float get_state_progress() const;
	//effects
	void add_effect(const std::string &name, const float ttl = -1);
	inline const bool has_effect(const std::string &name) const {
		return _effects.find(name) != _effects.end();
	}
	const float get_effect_timer(const std::string &name) const;
	void remove_effect(const std::string &name);
	
	virtual void add_damage(Object *from, const int hp, const bool emitDeath = true);
	void add_damage(Object *from, const bool emitDeath = true);

	virtual void emit(const std::string &event, Object * emitter = NULL);
	virtual void serialize(mrt::Serializator &s) const;
	virtual void deserialize(const mrt::Serializator &s);

	void serialize_all(mrt::Serializator &s) const;
	
	virtual void on_spawn();
	
	inline const bool rotating() const { return _direction_idx != _dst_direction; }

	virtual void calculate(const float dt);
	
	virtual const std::string getType() const;
	virtual const int getCount() const;

	const Object *get(const std::string &name) const;
	const bool has(const std::string &name) const;

	const float getWeaponRange(const std::string &weapon) const;

	const int get_target_position(v2<float> &relative_position, const v2<float> &target, const std::string &weapon) const;
	const int get_target_position(v2<float> &relative_position, const v2<float> &target, const float range) const;
	const int get_target_position(v2<float> &relative_position, const std::set<std::string> &targets, const std::string &weapon) const;
	const int get_target_position(v2<float> &relative_position, const std::set<std::string> &targets, const float range) const;

	void quantize_velocity();
	
	inline const Way& get_way() const { return _way; } 
	void set_way(const Way & way);
	const bool is_driven() const;
	
	const std::string get_nearest_waypoint(const std::string &classname) const;

	void set_zbox(const int z);

	virtual const bool detachVehicle();
	virtual const bool attachVehicle(Object *vehicle);

	const int get_children(const std::string &classname) const;
	void enumerate_objects(std::set<const Object *> &o_set, const float range, const std::set<std::string> *classfilter) const;

	static const bool check_distance(const v2<float> &map1, const v2<float>& map2, const int z, const bool use_pierceable_fixes);

	const bool ai_disabled() const;
	virtual const bool take(const BaseObject *obj, const std::string &type);

	Object * spawn(const std::string &classname, const std::string &animation, const v2<float> &dpos = v2<float>(), const v2<float> &vel = v2<float>(), const int z = 0);

	void invalidate() { set_sync(true); }

	//grouped object handling
	void pick(const std::string &name, Object *object); //picks up object and insert as group one, i.e. ctf-flag 
	Object *drop(const std::string &name, const v2<float> &dpos = v2<float>()); //pops object and inserts into world again
	
	Object *add(const std::string &name, const std::string &classname, const std::string &animation, const v2<float> &dpos, const GroupType type);
	Object *get(const std::string &name);
	void remove(const std::string &name);
	void group_emit(const std::string &name, const std::string &event);

	const bool get_render_rect(sdlx::Rect &src) const;
	
	bool is_subobject() const { return _parent != NULL; }
	
	inline const int get_slot() const { return _slot_id; }
	void set_slot(const int id);
	
	void update_outline(const bool hidden);

protected:

	//pathfinding

	struct Point {
		inline Point() : id(), parent(), g(0), h(0), dir(-1) {}
		v2<int> id, parent;
		int g, h, dir;
	};
	
	struct PD {
		int f;
		v2<int> id;
		inline PD(const int f, const v2<int> &id) : f(f), id(id) {}
		
		inline const bool operator<(const PD &other) const {
			return f > other.f;
		}
	};

	typedef std::set<v2<int> > CloseList;
	typedef std::priority_queue<PD> OpenList;
	typedef std::map<const v2<int>, Point> PointMap;


	void find_path(const v2<int> target, const int step);
	const bool find_path_done(Way &way);
	inline const bool calculating_path() const { return !_open_list.empty(); }

	void calculate_way_velocity();

	const bool get_nearest(const std::set<std::string> &classnames, const float range, v2<float> &position, v2<float> &velocity, const bool check_shooting_range) const;
	const Object * get_nearest_object(const std::set<std::string> &classnames, const float range, const bool check_shooting_range) const;
	
	void limit_rotation(const float dt, const float speed, const bool rotate_even_stopped, const bool allow_backward);
	
	void check_surface() const;
	
	virtual const bool skip_rendering() const;
	
	const sdlx::Surface * get_surface() const;
	const Matrix<int> &get_impassability_matrix() const;
	
	inline const AnimationModel * get_animation_model() const { return _model; }
	
	Object * _parent;
	
private: 
//pathfinding stuff
	void close(const v2<int>& vertex); 

	OpenList _open_list;
	PointMap _points;
	CloseList _close_list;
	v2<int> _end, _begin;
	int _step;
//end of pathfinding stuff

	struct Event : public mrt::Serializable {
		std::string name;
		bool repeat;
		std::string sound;
		float gain;
		bool played;
		mutable const Pose * cached_pose;
		
		Event();
		Event(const std::string name, const bool repeat, const std::string &sound, const float gain, const Pose * cached_pose = NULL);
		virtual void serialize(mrt::Serializator &s) const;
		virtual void deserialize(const mrt::Serializator &s);
	};
	
	void check_animation() const;
	void get_subobjects(std::set<Object *> &objects);
	void group_tick(const float dt);
	
	mutable const Animation *_animation;
	mutable const AnimationModel *_model;
	
	mutable const sdlx::Surface *_surface;
	
	sdlx::Surface *_fadeout_surface;
	int _fadeout_alpha;
	const sdlx::CollisionMap *_cmap;
	
	typedef std::deque<Event> EventQueue;
	EventQueue _events;
	
	typedef std::map<const std::string, float> EffectMap;
	EffectMap _effects;
	
	int _tw, _th;
	int _direction_idx, _directions_n;
	float _pos;

	//waypoints stuff
	Way _way;
	v2<float> _next_target, _next_target_rel;
	
	//rotation stuff
	float _rotation_time;	
	int _dst_direction;
	
	//grouped objects stuff
	typedef std::map<const std::string, Object *> Group;
	Group _group;
	
	int _slot_id;

	void set_sync(const bool sync);
	
	friend class IWorld;
	friend class ai::Buratino;
	friend class ai::Waypoints;
	using BaseObject::_position;
};



#endif

