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

#include "trooper.h"
#include "mrt/exception.h"
#include "registrar.h"
#include "config.h"

#include "ai/buratino.h" 

class AIMachinegunnerPlayer:  public Trooper, public ai::Buratino {
public: 
	AIMachinegunnerPlayer() :  Trooper("trooper", "machinegunner-bullet") {}
//	~AIMachinegunnerPlayer();
	virtual const std::string getType() const { return "machinegunner"; }
	virtual void on_spawn();
	virtual void calculate(const float dt);

	virtual Object * clone() const { return new AIMachinegunnerPlayer(*this); }

	virtual const std::string getWeapon(const int idx) const;
	virtual const int getWeaponAmount(const int idx) const;
	
private: 

};

const std::string AIMachinegunnerPlayer::getWeapon(const int idx) const {
	switch(idx) {
	case 0: 
		return "bullets:machinegunner";
	case 1:
		return std::string();
	default: 
		throw_ex(("weapon %d doesnt supported", idx));
	}
}

const int AIMachinegunnerPlayer::getWeaponAmount(const int idx) const{
	switch(idx) {
	case 0: 
	case 1:
		return -1;
	default: 
		throw_ex(("weapon %d doesnt supported", idx));
	}
}


void AIMachinegunnerPlayer::on_spawn() {
	addEnemyClass("fighting-vehicle");
	addEnemyClass("trooper");
	addEnemyClass("cannon");
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

	addBonusName("static-tank");
	addBonusName("static-launcher");
	addBonusName("static-shilka");
	addBonusName("static-mortar");
	//addBonusName("guided-missiles-item");
	//addBonusName("dumb-missiles-item");
	//addBonusName("nuke-missiles-item");
	//addBonusName("boomerang-missiles-item");
	//addBonusName("stun-missiles-item");
	//addBonusName("mines-item");

	ai::Buratino::on_spawn(this);
	Trooper::on_spawn();
}

void AIMachinegunnerPlayer::calculate(const float dt) {
	ai::Buratino::calculate(this, dt);
	
	GET_CONFIG_VALUE("objects.trooper.rotation-time", float, rt, 0.07);
	limit_rotation(dt, rt, true, false);
	update_state_from_velocity();	
}

REGISTER_OBJECT("machinegunner-player", AIMachinegunnerPlayer, ());
