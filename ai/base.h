#ifndef BTANKS_BASE_AI_H__
#define BTANKS_BASE_AI_H__

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

#include "export_btanks.h"
#include "object.h"
#include "traits.h"
#include <string>
#include <set>
#include "alarm.h"

namespace ai {
class BTANKSAPI Base : public virtual Object {
public: 
	Base();
	virtual ~Base();

	virtual void calculate(const float dt);
	virtual void onSpawn();
	
	virtual const std::string getWeapon(const int idx) const = 0;
	virtual const int getWeaponAmount(const int idx) const = 0;
	static const std::string convertName(const std::string &name);
	
protected: 
	void addEnemyClass(const std::string &classname);
	void addBonusName(const std::string &rname);
	const bool isEnemy(const Object *o) const;
	
	const bool checkTarget(const Object * target, const std::string &weapon) const;
private: 
	bool _active;
	Alarm _reaction_time, _refresh_path;
	ai::Traits _traits;
	std::set<std::string> _enemies, _bonuses;
	int _target_id;
	v2<int> _target_position;
	bool _enemy;
	int _pf_slice;
};
}

#endif

