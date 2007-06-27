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
#include "mrt/exception.h"
#include "resource_manager.h"
#include "config.h"

#include "ai/base.h" 

class AIMachinegunnerPlayer:  public Trooper, public ai::Base {
public: 
	AIMachinegunnerPlayer() :  Trooper("trooper", "machinegunner-bullet") {}
//	~AIMachinegunnerPlayer();
	virtual const std::string getType() const { return "machinegunner"; }
	virtual void onSpawn();
	virtual void calculate(const float dt);

	virtual Object * clone() const { return new AIMachinegunnerPlayer(*this); }

	virtual const std::string getWeapon(const int idx) const;
	virtual const int getWeaponAmount(const int idx) const;
	
private: 

};

const std::string AIMachinegunnerPlayer::getWeapon(const int idx) const {
	switch(idx) {
	case 0: 
		if (_object == "machinegunner-bullet")
			return "bullet";
		return Trooper::_object;
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


void AIMachinegunnerPlayer::onSpawn() {
	addEnemyClass("fighting-vehicle");
	addEnemyClass("trooper");
	addEnemyClass("kamikaze");
	addEnemyClass("boat");
	addEnemyClass("helicopter");
	addEnemyClass("watchtower");
	addEnemyClass("barrack");
	addEnemyClass("monster");
	
	addBonusName("heal");
	addBonusName("megaheal");
	//addBonusName("guided-missiles-item");
	//addBonusName("dumb-missiles-item");
	//addBonusName("nuke-missiles-item");
	//addBonusName("boomerang-missiles-item");
	//addBonusName("stun-missiles-item");
	//addBonusName("mines-item");

	ai::Base::onSpawn(this);
	Trooper::onSpawn();
}

void AIMachinegunnerPlayer::calculate(const float dt) {
	ai::Base::calculate(this, dt);
	
	GET_CONFIG_VALUE("objects.trooper.rotation-time", float, rt, 0.07);
	limitRotation(dt, rt, true, false);
	updateStateFromVelocity();	
}

REGISTER_OBJECT("machinegunner", AIMachinegunnerPlayer, ());
