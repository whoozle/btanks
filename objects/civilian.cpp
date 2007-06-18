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

#include "trooper.h"
#include "resource_manager.h"
#include "ai/waypoints.h"

class Civilian : public Trooper, public ai::Waypoints {
public: 
	Civilian(const std::string &classname) : Trooper(classname, std::string()) {} 
};

class AICivilian : public Civilian {
public: 
	AICivilian() : Civilian("civilian") {}
	void onSpawn() {
		_pose = "walk";
		Trooper::onSpawn();
		_avoid_obstacles = false;
		ai::Waypoints::onSpawn(this);
	}
	void calculate(const float dt) {
		ai::Waypoints::calculate(this, dt);
		updateStateFromVelocity();
	}
	Object *clone() const { return new AICivilian(*this); }
};

REGISTER_OBJECT("civilian-player", Civilian, ("player"));
REGISTER_OBJECT("civilian", AICivilian, ());
