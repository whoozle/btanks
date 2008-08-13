/* Battle Tanks Game
 * Copyright (C) 2006-2008 Battle Shilkas team
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

#include "shilka.h"
#include "mrt/exception.h"
#include "registrar.h"
#include "config.h"

#include "ai/buratino.h" 

class AIShilka:  public Shilka, public ai::Buratino {
public: 
	AIShilka(const std::string &classname) : Shilka(classname) {}
//	~AIShilka();
	virtual void on_spawn();
	virtual void calculate(const float dt);

	virtual Object * clone() const { return new AIShilka(*this); }

	virtual const std::string getWeapon(const int idx) const;
	virtual const int getWeaponAmount(const int idx) const;
private: 

};

const std::string AIShilka::getWeapon(const int idx) const {
	switch(idx) {
	case 0: 
		if (has_effect("dispersion")) {
			return "bullets:dispersion";
		} else if (has_effect("ricochet")) {
			return "bullets:ricochet";
		}
		return "bullet";
	case 1:
		if (has_effect("dirt")) {
			return "bullets:dirt";
		} 	
		return "bullet";
	default: 
		throw_ex(("weapon %d doesnt supported", idx));
	}
}

const int AIShilka::getWeaponAmount(const int idx) const{
	switch(idx) {
	case 0: 
		return -1;
	case 1:
		{
		int n = get("mod")->getCount();
		if (n == -1 || n > 0)
			return n;
		}
		//place something here
		return -1;
	default: 
		throw_ex(("weapon %d doesnt supported", idx));
	}
}


void AIShilka::on_spawn() {
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

	//primary weapon	
	addBonusName("dispersion-bullets-item");
	addBonusName("ricochet-bullets-item");

	//secondary weapon
	addBonusName("machinegunner-item");
	addBonusName("thrower-item");
	addBonusName("mines-item");
	addBonusName("nuke-missiles-item");

	ai::Buratino::on_spawn(this);
	Shilka::on_spawn();
}

void AIShilka::calculate(const float dt) {
	ai::Buratino::calculate(this, dt);
	
	GET_CONFIG_VALUE("objects.shilka.rotation-time", float, rt, 0.05);
	limit_rotation(dt, rt, true, false);
	update_state_from_velocity();	
}

REGISTER_OBJECT("shilka", AIShilka, ("fighting-vehicle"));
REGISTER_OBJECT("static-shilka", AIShilka, ("vehicle"));
