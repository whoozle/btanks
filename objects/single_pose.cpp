
/* Battle Tanks Game
 * Copyright (C) 2006-2009 Battle Tanks team
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

#include "object.h"
#include "registrar.h"

class SinglePose : public Object {
public:
	SinglePose(const std::string &pose) : 
		Object("single-pose"), _pose(pose) {
			impassability = 0;
			hp = -1;
		}

	virtual Object * clone() const;
	virtual void tick(const float dt);
	virtual void on_spawn();
	virtual void render(sdlx::Surface &surface, const int x, const int y);

	virtual void serialize(mrt::Serializator &s) const {
		Object::serialize(s);
		s.add(_pose);
	}

	virtual void deserialize(const mrt::Serializator &s) {
		Object::deserialize(s);
		s.get(_pose);
	}

private:
	std::string _pose;
};

void SinglePose::render(sdlx::Surface &surface, const int x, const int y) {
	if (_variants.has("no-directions"))
		set_direction(0);
	Object::render(surface, x, y);
}


void SinglePose::tick(const float dt) {
	Object::tick(dt);
	if (get_state().empty()) {	
		//LOG_DEBUG(("over"));
		emit("death", this);
	}
}

void SinglePose::on_spawn() {
	//LOG_DEBUG(("single-pose: play('%s', %s)", _pose.c_str(), _repeat?"true":"false"));
	play(_pose, !_variants.has("once"));
	if (_variants.has("play-start")) {
		play_now("start");
	}
}


Object* SinglePose::clone() const  {
	return new SinglePose(*this);
}

REGISTER_OBJECT("single-pose", SinglePose, ("main"));
REGISTER_OBJECT("broken-object", SinglePose, ("broken"));
REGISTER_OBJECT("outline", SinglePose, ("main"));
REGISTER_OBJECT("eternal-flame", SinglePose, ("burn"));
REGISTER_OBJECT("helmet", SinglePose, ("hold"));
