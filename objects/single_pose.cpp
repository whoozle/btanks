
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

#include "object.h"
#include "resource_manager.h"

class SinglePose : public Object {
public:
	SinglePose(const std::string &pose, const bool repeat, const bool no_directions = false, const bool play_start = false) : 
		Object("single-pose"), _pose(pose), _repeat(repeat), _no_dir(no_directions), _play_start(play_start)  {
			impassability = 0;
		}

	virtual Object * clone() const;
	virtual void tick(const float dt);
	virtual void onSpawn();
	virtual void render(sdlx::Surface &surface, const int x, const int y);

	virtual void serialize(mrt::Serializator &s) const {
		Object::serialize(s);
		s.add(_pose);
		s.add(_repeat);
		s.add(_no_dir);
		s.add(_play_start);
	}

	virtual void deserialize(const mrt::Serializator &s) {
		Object::deserialize(s);
		s.get(_pose);
		s.get(_repeat);
		s.get(_no_dir);
		s.get(_play_start);
	}

private:
	std::string _pose;
	bool _repeat, _no_dir, _play_start;
};

void SinglePose::render(sdlx::Surface &surface, const int x, const int y) {
	if (_no_dir)
		setDirection(0);
	Object::render(surface, x, y);
}


void SinglePose::tick(const float dt) {
	Object::tick(dt);
	if (getState().empty()) {	
		//LOG_DEBUG(("over"));
		emit("death", this);
	}
}

void SinglePose::onSpawn() {
	//LOG_DEBUG(("single-pose: play('%s', %s)", _pose.c_str(), _repeat?"true":"false"));
	play(_pose, _repeat);
	if (_play_start) {
		playNow("start");
	}
}


Object* SinglePose::clone() const  {
	return new SinglePose(*this);
}

REGISTER_OBJECT("single-pose", SinglePose, ("main", true));
REGISTER_OBJECT("broken-object", SinglePose, ("broken", true));
REGISTER_OBJECT("single-pose-with-start", SinglePose, ("main", true, false, true));
REGISTER_OBJECT("single-pose-once", SinglePose, ("main", false));
REGISTER_OBJECT("single-pose-no-directions", SinglePose, ("main", true, true));
REGISTER_OBJECT("single-pose-once-no-directions", SinglePose, ("main", false, true));

REGISTER_OBJECT("outline", SinglePose, ("main", true));
REGISTER_OBJECT("missile-launch", SinglePose, ("launch", false));
REGISTER_OBJECT("train-smoke", SinglePose, ("main", false));
