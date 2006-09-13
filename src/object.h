#ifndef __BTANKS_ANIMATED_OBJECT__
#define __BTANKS_ANIMATED_OBJECT__
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

#include "base_object.h"
#include <string>
#include <map>
#include "math/v3.h"
#include "object_common.h"

namespace sdlx {
	class Surface;
}

class AnimationModel;
class Pose;

class Object : public BaseObject {
public:
	std::string animation;
	float fadeout_time;

	Object(const std::string &classname);
	void init(const std::string &model, const std::string &surface, const int tile_w, const int tile_h);
	void init(const Object *other);
	virtual Object * clone() const;

	void setDirection(const int dir);
	const int getDirection() const;
	
	virtual void tick(const float dt);
	virtual void render(sdlx::Surface &surface, const int x, const int y);
	void renderCopy(sdlx::Surface &surface);
	
	void play(const std::string &id, const bool repeat = false);
	void playNow(const std::string &id);
	void cancel();
	void cancelRepeatable();
	void cancelAll();
	const std::string& getState() const;

	virtual void emit(const std::string &event, BaseObject * emitter = NULL);
	virtual void serialize(mrt::Serializator &s) const;
	virtual void deserialize(const mrt::Serializator &s);

	virtual void onSpawn();
	void setup(const std::string &animation); //do not use it, needed for resman

protected:

	Object * spawn(const std::string &classname, const std::string &animation, const v3<float> &dpos, const v3<float> &vel);
	Object * spawnGrouped(const std::string &classname, const std::string &animation, const v3<float> &dpos, const GroupType type);

	const bool getNearest(const std::string &classname, v3<float> &position, v3<float> &velocity, Way * way = NULL) const;
	
	void setWay(const Way & way);
	const bool isDriven() const;

	void limitRotation(const float dt, const int dirs, const float speed, const bool rotate_even_stopped, const bool allow_backward);
	
	void add(const std::string &name, Object *obj);
	Object *get(const std::string &name);
	const Object *get(const std::string &name) const;
	void groupEmit(const std::string &name, const std::string &event);

private: 
	struct Event : public mrt::Serializable {
		std::string name;
		bool repeat;
		
		Event();
		Event(const std::string name, const bool repeat);
		virtual void serialize(mrt::Serializator &s) const;
		virtual void deserialize(const mrt::Serializator &s);
	};
	
	
	const AnimationModel *_model;
	std::string _model_name;
	const sdlx::Surface *_surface;
	std::string _surface_name;
	
	typedef std::deque<Event> EventQueue;
	EventQueue _events;
	
	int _tw, _th;
	int _direction_idx;
	float _pos;

	//waypoints stuff
	Way _way;
	
	//rotation stuff
	float _rotation_time;	
	int _dst_direction;
	
	//grouped objects stuff
	typedef std::map<const std::string, Object *> Group;
	Group _group;
};



#endif

