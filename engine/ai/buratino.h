#ifndef BTANKS_BASE_AI_H__
#define BTANKS_BASE_AI_H__

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

#include "export_btanks.h"
#include "object.h"
#include "traits.h"
#include <string>
#include <set>
#include "alarm.h"
#include "synchronizable.h"

namespace ai {
class BTANKSAPI Buratino : public ai::Synchronizable {
public: 
	Buratino();
	virtual ~Buratino();

	virtual void calculate(Object *obj, const float dt);
	virtual void on_spawn(const Object *obj);
	
	virtual const std::string getWeapon(const int idx) const = 0;
	virtual const int getWeaponAmount(const int idx) const = 0;
	static const std::string convertName(const std::string &name);

	const bool active() const;
	const float getWeaponRange(const Object *object) const;
	
	static const float getFirePower(const Object *o, ai::Traits &traits);
	
protected: 
	virtual void calculateCloseCombat(Object *obj, const Object *target, const float range, const bool dumb);
	void processPF(Object *object);

	void addEnemyClass(const std::string &classname);
	void addBonusName(const std::string &rname);
	const bool isEnemy(const Object *o) const;
	
	const bool checkTarget(const Object *obj, const Object * target, const std::string &weapon) const;
	const Object * findTarget(const Object *src, const std::set<std::string> &enemies, const std::set<std::string> &bonuses, ai::Traits &traits, const std::set<int> &skip_objects) const;

private: 

	Alarm _reaction_time, _refresh_path;
	ai::Traits _traits;
	std::set<std::string> _enemies, _bonuses;
	std::set<int> _skip_objects;
	int _target_id;
	v2<float> _target_position;
	bool _enemy;
	
	int _pf_slice;
	int _target_dir;
};
}

#endif

