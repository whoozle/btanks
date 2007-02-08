#ifndef __BTANKS_ANIMATED_OBJECT__
#define __BTANKS_ANIMATED_OBJECT__

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

#include "base_object.h"
#include "alarm.h"
#include <string>
#include <map>
#include <set>
#include <queue>
#include "math/v2.h"
#include "object_common.h"

namespace sdlx {
	class Rect;
	class Surface;
	class CollisionMap;
}

class AnimationModel;
class Animation;
class Pose;

class Object : public BaseObject {
public:
	std::string registered_name; //for resource manager only. dont use it.
	
	std::string animation;
	float fadeout_time;

	Object(const std::string &classname);
	~Object();
	
	//void init(const std::string &model, const std::string &surface, const int tile_w, const int tile_h);
	void init(const Animation *other);
	virtual Object * clone() const;

	virtual void setDirection(const int dir);
	const int getDirection() const;
	const int getDirectionsNumber() const;
	void setDirectionsNumber(const int dirs);
	
	virtual void tick(const float dt);
	virtual void render(sdlx::Surface &surface, const int x, const int y);
	const bool collides(const Object *other, const int x, const int y, const bool hidden_by_other = false) const;
	const bool collides(const sdlx::CollisionMap *other, const int x, const int y, const bool hidden_by_other = false) const;

	// animation:
	void play(const std::string &id, const bool repeat = false);
	void playNow(const std::string &id);
	void cancel();
	void cancelRepeatable();
	void cancelAll();
	inline const std::string& getState() const {
		static const std::string empty;
		if (_events.empty())
			return empty;
		return _events.front().name;
	}
	const float getStateProgress() const;
	//effects
	void addEffect(const std::string &name, const float ttl = -1);
	inline const bool isEffectActive(const std::string &name) const {
		return _effects.find(name) != _effects.end();
	}
	const float getEffectTimer(const std::string &name) const;
	void removeEffect(const std::string &name);
	
	virtual void addDamage(Object *from, const int hp, const bool emitDeath = true);
	void addDamage(Object *from, const bool emitDeath = true);

	virtual void emit(const std::string &event, Object * emitter = NULL);
	virtual void serialize(mrt::Serializator &s) const;
	virtual void deserialize(const mrt::Serializator &s);

	virtual void onSpawn();
	void setup(const std::string &animation); //do not use it, needed for resman
	
	const bool rotating() const { return _direction_idx != _dst_direction; }

	virtual void calculate(const float dt);
	
	virtual const std::string getType() const;
	virtual const int getCount() const;

	const Object *get(const std::string &name) const;
	const bool has(const std::string &name) const;

	const float getWeaponRange(const std::string &weapon) const;

	const bool getTargetPosition(v2<float> &relative_position, const v2<float> &target, const std::string &weapon) const;
	const int getTargetPosition(v2<float> &relative_position, const std::set<std::string> &targets, const std::string &weapon) const;

	void quantizeVelocity();
	
	const Way& getWay() const { return _way; } 
	
	const std::string getNearestWaypoint(const std::string &classname) const;
protected:
	//pathfinding

	struct Point {
		Point() : dir(-1) {}
		v2<int> id, parent;
		int g, h, dir;
	};
	
	struct PD {
		int f;
		v2<int> id;
		PD(const int f, const v2<int> &id) : f(f), id(id) {}
		
		inline const bool operator<(const PD &other) const {
			return f > other.f;
		}
	};

	typedef std::set<v2<int> > CloseList;
	typedef std::priority_queue<PD> OpenList;
	typedef std::map<const v2<int>, Point> PointMap;


	void findPath(const v2<int> target, const int step);
	const bool findPathDone(Way &way);
	const bool calculatingPath() const { return !_open_list.empty(); }
	virtual const int getPenalty(const int map_im, const int obj_im) const;

	//grouped object handling
	void add(const std::string &name, Object *obj);
	Object *get(const std::string &name);
	void remove(const std::string &name);
	void groupEmit(const std::string &name, const std::string &event);

	const bool getRenderRect(sdlx::Rect &src) const;

	void calculateWayVelocity();

	Object * spawn(const std::string &classname, const std::string &animation, const v2<float> &dpos = v2<float>::empty, const v2<float> &vel = v2<float>::empty, const int z = 0);
	Object * spawnGrouped(const std::string &classname, const std::string &animation, const v2<float> &dpos, const GroupType type);

	const bool old_findPath(const v2<float> &position, Way &way) const;
	const bool old_findPath(const Object *target, Way &way) const;

	const Object* getNearestObject(const std::string &classname) const;
	const Object* getNearestObject(const std::set<std::string> &classnames) const;
	const bool getNearest(const std::string &classname, v2<float> &position, v2<float> &velocity, Way * way = NULL) const;
	const bool getNearest(const std::set<std::string> &targets, v2<float> &position, v2<float> &velocity) const;
	const bool getNearest(const std::set<std::string> &classnames, const float range, v2<float> &position, v2<float> &velocity) const;
	
	void setWay(const Way & way);
	const bool isDriven() const;

	void limitRotation(const float dt, const float speed, const bool rotate_even_stopped, const bool allow_backward);
	
	void checkSurface();
	
	virtual const bool skipRendering() const;
	
	const sdlx::Surface * getSurface() const;

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
		bool played;
		mutable const Pose * cached_pose;
		
		Event();
		Event(const std::string name, const bool repeat, const std::string &sound, const Pose * cached_pose = NULL);
		virtual void serialize(mrt::Serializator &s) const;
		virtual void deserialize(const mrt::Serializator &s);
	};
	
	
	const AnimationModel *_model;
	std::string _model_name;
	const sdlx::Surface *_surface;
	sdlx::Surface *_fadeout_surface;
	int _fadeout_alpha;
	const sdlx::CollisionMap *_cmap;
	std::string _surface_name;
	
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
	typedef std::map<const std::string, int> Group;
	Group _group;
	
	Alarm _blinking;
	
	friend class IWorld;
};



#endif

