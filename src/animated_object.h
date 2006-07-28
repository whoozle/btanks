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

#include "object.h"
#include <deque>
#include <string>
#include "v3.h"

namespace sdlx {
	class Surface;
}

class AnimationModel;
class Pose;

class AnimatedObject : public Object {
public:
	AnimatedObject(const std::string &classname);
	void init(AnimationModel *model, sdlx::Surface *surface, const int tile_w, const int tile_h);
	void init(const AnimatedObject &o);
	virtual Object * clone(const std::string &opt) const;

	void setDirection(const int dir);
	const int getDirection() const;
	
	virtual void tick(const float dt);
	void render(sdlx::Surface &surface, const int x, const int y);
	
	void play(const std::string &id, const bool repeat = false);
	void playNow(const std::string &id);
	void cancel();
	void cancelRepeatable();
	void cancelAll();
	const std::string getState() const;

	virtual void serialize(mrt::Serializator &s) const;
	virtual void deserialize(const mrt::Serializator &s);
private: 
	struct Event {
		std::string name;
		bool repeat;
		const Pose * pose;
		Event(const std::string name, const bool repeat, const Pose *pose) : name(name), repeat(repeat), pose(pose) {}
	};

	AnimationModel *_model;
	sdlx::Surface *_surface;
	
	typedef std::deque<Event> EventQueue;
	EventQueue _events;
	
	int _tw, _th;
	int _direction_idx;
	float _pos;
};

#endif

