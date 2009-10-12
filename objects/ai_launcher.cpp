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

#include "launcher.h"
#include "mrt/exception.h"
#include "registrar.h"
#include "config.h"

#include "ai/buratino.h" 

class AILauncher:  public Launcher, public ai::Buratino {
public: 
	AILauncher(const std::string &classname) : Launcher(classname) {}
//	~AILauncher();
	virtual void on_spawn();
	virtual void calculate(const float dt);

	virtual Object * clone() const { return new AILauncher(*this); }

	virtual const std::string getWeapon(const int idx) const;
	virtual const int getWeaponAmount(const int idx) const;
private: 

};

const std::string AILauncher::getWeapon(const int idx) const {
	switch(idx) {
	case 0: 
	case 1:
		return get( idx ? "alt-mod": "mod")->getType();
	default: 
		throw_ex(("weapon %d doesnt supported", idx));
	}
}

const int AILauncher::getWeaponAmount(const int idx) const{
	switch(idx) {
	case 0: 
	case 1:
		return get( idx ? "alt-mod": "mod")->getCount();
	default: 
		throw_ex(("weapon %d doesnt supported", idx));
	}
}


void AILauncher::on_spawn() {
	addEnemyClass("fighting-vehicle");
	addEnemyClass("cannon");
	addEnemyClass("trooper");
	addEnemyClass("kamikaze");
	addEnemyClass("boat");
	addEnemyClass("helicopter");
	addEnemyClass("watchtower");
	addEnemyClass("barrack");
	addEnemyClass("monster");
	
	addBonusName("teleport");
	addBonusName("ctf-flag");

	addBonusName("heal");
	addBonusName("megaheal");
	addBonusName("guided-missiles-item");
	addBonusName("dumb-missiles-item");
	addBonusName("nuke-missiles-item");
	addBonusName("boomerang-missiles-item");
	addBonusName("stun-missiles-item");
	addBonusName("mines-item");

	addBonusName("machinegunner-item");
	addBonusName("thrower-item");

	ai::Buratino::on_spawn(this);
	Launcher::on_spawn();
}

void AILauncher::calculate(const float dt) {
	ai::Buratino::calculate(this, dt);
	
	GET_CONFIG_VALUE("objects.launcher.rotation-time", float, rt, 0.07);
	limit_rotation(dt, rt, true, false);
	update_state_from_velocity();	
}

REGISTER_OBJECT("launcher", AILauncher, ("fighting-vehicle"));
REGISTER_OBJECT("static-launcher", AILauncher, ("vehicle"));
