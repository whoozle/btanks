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

#include "launcher.h"
#include "mrt/exception.h"
#include "resource_manager.h"
#include "config.h"

#include "ai/base.h" 

class AILauncher:  public Launcher, public ai::Base {
public: 
	AILauncher() : Object("player") {}
//	~AILauncher();
	virtual void onSpawn();
	virtual void calculate(const float dt);

	virtual Object * clone() const { return new AILauncher(*this); }
private: 

};

void AILauncher::onSpawn() {
	ai::Base::onSpawn();
	Launcher::onSpawn();
}

void AILauncher::calculate(const float dt) {
	ai::Base::calculate(dt);
	
	GET_CONFIG_VALUE("objects.launcher.rotation-time", float, rt, 0.07);
	limitRotation(dt, rt, true, false);
}

REGISTER_OBJECT("ai-launcher", AILauncher, ());
