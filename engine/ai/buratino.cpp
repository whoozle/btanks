
/* Battle Tanks Game
 * Copyright (C) 2006-2008 Battle Tanks team
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
#include "buratino.h"
#include <assert.h>
#include "world.h"
#include "config.h"

#include "math/unary.h"
#include "player_manager.h"
#include "game_monitor.h"
#include "mrt/random.h"
#include "math/binary.h"
#include "vehicle_traits.h"
#include "tmx/map.h"
#include "special_zone.h"
#include "team.h"

using namespace ai;

Buratino::Buratino() : _reaction_time(true), _refresh_path(false), _target_id(-1), _target_dir(-1) {}

const bool Buratino::active() const {
	return !PlayerManager->isClient();
}

Buratino::~Buratino() {
	if (!active())
		return;
	
	if (!_traits.empty())
		LOG_DEBUG(("traits: \n%s", _traits.save().c_str()));
}

void Buratino::addEnemyClass(const std::string &classname) {
	_enemies.insert(classname);
}

void Buratino::addBonusName(const std::string &rname) {
	_bonuses.insert(rname);
}

void Buratino::processPF(Object *object) {
	if (object->calculatingPath()) {
		Way way;
		int n = 1;
		bool found;
		while(! (found = object->findPathDone(way)) && n < _pf_slice)
			++n;
		
		if (found) {
			//LOG_DEBUG(("n = %d", n));
			if (!way.empty()) {
				object->setWay(way);
				_skip_objects.clear();
			} else {
				LOG_DEBUG(("no path, adding %d to targets black list ", _target_id));
				_skip_objects.insert(_target_id);
			}
		}
	} else  {
		//LOG_DEBUG(("idle"));
		
	}
}


void Buratino::onSpawn(const Object *object) {
	if (!active())
		return;
	
	const std::string vehicle = object->getType();
	if (vehicle.empty())
		throw_ex(("vehicle MUST provide its type"));
	
	LOG_DEBUG(("spawning as '%s'", vehicle.c_str()));
	if (_enemies.empty() && _bonuses.empty()) 
		throw_ex(("vehicle had not provided enemies/bonuses"));
	
	float rt;
	Config->get("objects.ai-" + object->registered_name + ".reaction-time", rt, 0.1f);
	float rpi = 2.0f;

	mrt::randomize(rt, rt/10);
	_reaction_time.set(rt);

	mrt::randomize(rpi, rpi/10);
	_refresh_path.set(rpi);
	
	Config->get("objects.ai-" + vehicle + ".pathfinding-slice", _pf_slice, 10);
}

const bool Buratino::isEnemy(const Object *o) const {
	return _enemies.find(o->classname) != _enemies.end();
}

const std::string Buratino::convertName(const std::string &weapon) {
	std::string wc, wt;
	std::string::size_type p;
	if ((p = weapon.rfind(':')) != std::string::npos) {
		wc = weapon.substr(0, p);
		wt = weapon.substr(p + 1);
	} else {
		wt = weapon;
	}
	if (wc.empty()) 
		return wt;
	return wt + "-" + wc.substr(0, wc.size() - 1);
}


const bool Buratino::checkTarget(const Object *object, const Object * target, const std::string &weapon) const {
	if (!isEnemy(target))
		return false;
	
	v2<float> pos = object->getRelativePosition(target);
	
	std::string wc, wt;
	{
		std::string::size_type p;
		if ((p = weapon.rfind(':')) != std::string::npos) {
			wc = weapon.substr(0, p);
			wt = weapon.substr(p + 1);
		} else {
			wc = weapon;
		}
	}

	bool codir, codir1;
	{
		v2<float> d(pos);
		d.normalize();
		int dir = d.getDirection(object->getDirectionsNumber()) - 1;
		codir = dir == object->getDirection();
		int dd = math::abs(dir - object->getDirection());
		codir1 = codir || dd == 1 || dd == (object->getDirectionsNumber() - 1);
	}

	//LOG_DEBUG(("checking target(%s/%s): %g %g codir: %c, codir1: %c", wc.c_str(), wt.c_str(), pos.x, pos.y, codir?'+':'-', codir1?'+':'-'));
	
	if (wc == "missiles" || wc == "bullets" || wc == "bullet") {
		if (codir)
			return true;
		if ((wt == "guided" && codir1) || wt == "dispersion")
			return true;
		if (wt == "boomerang")
			return true;
	} else if (wc == "mines") {
		if (!object->_velocity.is0())
			return true;
	}
	return false;
}

void Buratino::calculateCloseCombat(Object *object, const Object *target, const float range, const bool dumb) {
	assert(object != NULL);
	assert(target != NULL);
	
	//LOG_DEBUG(("close combat with %s, range: %g, dumb: %c", target->animation.c_str(), range, dumb?'+':'-'));

	if (!dumb) {
		_target_dir = object->getTargetPosition(_target_position, object->getRelativePosition(target), range);
		if (_target_dir >= 0)
			Map->add(_target_position, object->getCenterPosition());
	} 

	object->_velocity = Map->distance(object->getCenterPosition(), _target_position);
	
	//LOG_DEBUG(("object velocity: %g,%g, target dir: %d", object->_velocity.x, object->_velocity.y, _target_dir));
	
	if (_target_dir >= 0) {
		int dirs = object->getDirectionsNumber();
		if (object->_velocity.length() >= 9) {
			object->quantizeVelocity();
			object->_direction.fromDirection(object->getDirection(), dirs);
		} else {
			object->_velocity.clear();
			object->setDirection(_target_dir);
			//LOG_DEBUG(("%d", _target_dir));
			object->_direction.fromDirection(_target_dir, dirs);
			std::string weapon1 = getWeapon(0), weapon2 = getWeapon(1);
			object->_state.fire = checkTarget(object, target, weapon1);
			object->_state.alt_fire = checkTarget(object, target, weapon2);
			//LOG_DEBUG(("firing: %c%c", object->_state.fire?'+':'-', object->_state.alt_fire?'+':'-'));
		}
	} else {
		object->_velocity.clear();
	}
}

const float Buratino::getWeaponRange(const Object *object) const {
	std::string weapon1 = getWeapon(0), weapon2 = getWeapon(1);
	
	float range = 0;
	if (!weapon1.empty()) {
		range = math::max(range, object->getWeaponRange(convertName(weapon1)));
	}
	if (!weapon2.empty()) {
		range = math::max(range, object->getWeaponRange(convertName(weapon2)));
	}
	return range;	
}

void Buratino::calculate(Object *object, const float dt) {
	if (object->aiDisabled()) {
		return;
	}
	
	if (!active()) {
		if (object->isDriven()) 
			object->calculateWayVelocity();
		else 
			object->Object::calculate(dt);
		object->updateStateFromVelocity();
		return;
	}

	const bool racing = object->getVariants().has("racing");
	const bool refresh_path = !racing && _refresh_path.tick(dt) && object->isDriven();
	const bool dumb = !_reaction_time.tick(dt);
	const Object *target = NULL;
	
	std::string weapon1, weapon2;
	int amount1, amount2;

	if (dumb) {
		if (_target_dir >= 0) {
			if (target == NULL)
				target = World->getObjectByID(_target_id);
			if (target == NULL)
				goto gogogo;
				
			//processPF(object);
			calculateCloseCombat(object, target, getWeaponRange(object), true);
			goto skip_calculations;
		}
		goto gogogo;
	}
	

	weapon1 = getWeapon(0), weapon2 = getWeapon(1);
	amount1 = getWeaponAmount(0), amount2 = getWeaponAmount(1);
	
	if (target == NULL)
		target = World->getObjectByID(_target_id);

	if (amount1 < 0) 
		amount1 = 50;
	if (amount2 < 0) 
		amount2 = 50; //infinite amount
	
	if (target != NULL) {
		if (!weapon1.empty())
			object->_state.fire = checkTarget(object, target, weapon1);
		if (!weapon2.empty())
			object->_state.alt_fire = checkTarget(object, target, weapon2);

		float range = getWeaponRange(object);
		
		v2<float> dpos = object->getRelativePosition(target);
		if (_enemy && dpos.length() <= range) {
			//processPF(object);
			calculateCloseCombat(object, target, range, false);
			
			if (_target_dir >= 0) {
				if (object->isDriven())
					object->setWay(Way());
			}
		} else {
			_target_dir = -1;
		}
	}
	
	if (racing) {
		Way way;
		if (object->calculatingPath()) {
			goto gogogo;
		}
		if (!object->isDriven()) {
			int slot_id = PlayerManager->getSlotID(object->getID());
			if (slot_id <= 0) 
				throw_ex(("ai in racing mode cannot operate without slot."));
		
			PlayerSlot &slot = PlayerManager->getSlot(slot_id);
			const SpecialZone &zone = PlayerManager->getNextCheckpoint(slot);
			v3<int> position = zone.getPlayerPosition(slot_id);
		
			object->findPath(v2<int>(position.x, position.y), 24);
		}
	}
		
	target = findTarget(object, 
		(amount1 > 0 || amount2 > 0)?_enemies:std::set<std::string>(), 
		object->getVariants().has("no-bonuses")?std::set<std::string>():_bonuses, 
		_traits, _skip_objects);
	
	if (target != NULL) {
		if ( ((refresh_path && isEnemy(target)) || target->getID() != _target_id)) {
			_target_id = target->getID();
			_enemy = isEnemy(target);
			v2<int> target_position;
			target->getCenterPosition(target_position);
/*
			if (_enemy && !weapon1.empty()) {
				v2<float> r;
				if (object->getTargetPosition(r, target->getPosition(), convertName(weapon1)))
					_target_position = r.convert<int>();
			}
*/		
			LOG_DEBUG(("%d: %s: next target: %s at %d,%d", 
				object->getID(), object->animation.c_str(), target->animation.c_str(), target_position.x, target_position.y));
			object->findPath(target_position, 24);
			_refresh_path.reset();
		
			//Way way;
			//if (!old_findPath(target, way))
			//	LOG_WARN(("no way"));
			//else setWay(way);
		}
	} else if (!object->isDriven()) {
		//LOG_DEBUG(("%d:%s idle", object->_id, object->animation.c_str()));
		object->_velocity.clear();
	}

	//2 fire or not 2 fire.


	
	//LOG_DEBUG(("w1: %s", getWeapon(0).c_str()));
	//LOG_DEBUG(("w2: %s", getWeapon(1).c_str()));
	
	//bool driven = isDriven();
	
	//LOG_DEBUG(("calculating: %c, driven: %c", calculating?'+':'-', driven?'+':'-'));
	gogogo:
	
	processPF(object);
	
	object->calculateWayVelocity();

skip_calculations: 	
	if (target != NULL) {
		if (!weapon1.empty() && !object->_state.fire)
			object->_state.fire = checkTarget(object, target, weapon1);
		if (!weapon2.empty() && !object->_state.alt_fire)
			object->_state.alt_fire = checkTarget(object, target, weapon2);
	}
	object->updateStateFromVelocity();
}

const Object * Buratino::findTarget(const Object *src, const std::set<std::string> &enemies, const std::set<std::string> &bonuses, ai::Traits &traits, const std::set<int> &skip_objects) const {
	if (src->getVariants().has("racing"))
		return NULL;

	if (src->has("#ctf-flag")) {
		Team::ID team = Team::get_team(src->get("#ctf-flag"));
		if (team != Team::Red && team != Team::Green) 
			throw_ex(("flag team must be red or green"));
		int base_id = GameMonitor->getBase(team == Team::Red? Team::Green: Team::Red);
		Object *base = World->getObjectByID(base_id);
		if (base != NULL) {
			return base;
		}
	}
	
	if (src->getType().empty())
		throw_ex(("findTarget source must always provide its type"));
	
	const Object *result = NULL;
	float result_value = 0;
	std::set<const Object *> objects;
	{
		float range;
		Config->get("objects." + src->registered_name + ".range", range, 640.0f);
		World->enumerateObjects(objects, src, range, NULL);
	}
	
	for(std::set<const Object *>::const_iterator i = objects.begin(); i != objects.end(); ++i) {
		const Object *o = *i;
		if (o->impassability == 0 || o->_id == src->_id ||
			!ZBox::sameBox(src->getZ(), o->getZ()) || 
			o->hasSameOwner(src) || 
			o->has_effect("invulnerability") || 
			skip_objects.find(o->getID()) != skip_objects.end()
			)
			continue;
		
		const bool enemy = enemies.find(o->classname) != enemies.end();
		if (enemy && o->hp < 0) 
			continue; //invulnerable enemy
		
		const bool bonus = bonuses.find(o->registered_name) != bonuses.end() || bonuses.find(o->classname) != bonuses.end();
		//LOG_DEBUG(("object: %s", o->registered_name.c_str()));
		if (!enemy && !bonus)
			continue;

		//bonus!
		int min = 0, max = 0;
		std::string mod_type = o->classname;
		if (mod_type == "teleport") {
			v2<float> dpos = src->getRelativePosition(o);
			if (dpos.quick_length() < ((o->size.x + o->size.y) * 141 / 100)) 
				continue;
		}
		if (!o->getType().empty()) 
			mod_type += ":" + o->getType();
		if (o->has_effect("invulnerability"))
			continue;
		
		if (o->classname == "missiles" || o->classname == "mines") {
			if (src->has("mod")) {
				const Object *mod = src->get("mod");
				if (mod->getType() == mod_type)
					min = mod->getCount();
			}
	
			if (src->has("alt-mod")) {
				const Object *mod = src->get("alt-mod");
				if (mod->getType() == mod_type)
					min = mod->getCount();
			}
			assert(min >= 0);
			int max_v;
			VehicleTraits::getWeaponCapacity(max, max_v, src->getType(), o->classname, o->getType());
			if (min > max)
				min = max;
		} else if (o->classname == "heal") {
			min = src->hp;
			max = src->max_hp;
		} else if (o->classname == "effects" || o->classname == "mod") {
			max = 1;
		} else if (o->classname == "teleport") {
			max = 1;
		} else if (o->classname == "vehicle") {
			max = 1;
		} else if (o->classname == "ctf-flag") {
			PlayerSlot *slot = PlayerManager->getSlotByID(src->getID());
			if (slot == NULL)
				continue;
			Team::ID flag_team = Team::get_team(o);
			if (flag_team == slot->team) {
				const Object *base = World->getObjectByID(o->getSummoner());
				if (base == NULL) {
					LOG_WARN(("could not find base #%d for %s", o->getSummoner(), o->animation.c_str()));
					continue;
				}

				v2<float> dpos = o->getRelativePosition(base);
				if (dpos.quick_length() < o->size.x * o->size.y / 4) {
					//my flag is on the base
					continue;
				}

			} 
			min = 0;
			max = 1;
		}
		float value = 0;
		const std::string type = o->getType();

		if (enemy) {
			value = traits.get("enemy", type.empty()?o->classname:type, 1000.0, 1000.0);
		} else if (bonus) {
			if (max == 0) {
				LOG_WARN(("cannot determine bonus for %s", o->registered_name.c_str()));
				continue;
			}
			value = traits.get("value", mod_type, 1.0, 1000.0) * (max - min);
		} else assert(0);
				
		if (enemy) {
			value *= (getFirePower(src, traits) + 1) / (getFirePower(o, traits) + 1);
		}
		value /= (src->_position.distance(o->_position));
		//LOG_DEBUG(("item: %s, value: %g", o->registered_name.c_str(), value));
		//find most valuable item.
		if (value > result_value) {
			result = o;
			result_value = value;
		}
	}
	return result;
}

const float Buratino::getFirePower(const Object *o, ai::Traits &traits) {
	float value = 0;
	if (o->has("mod")) {
		const Object *mod = o->get("mod");
		int c = mod->getCount();
		const std::string& type = mod->getType();
		if (c > 0 && !type.empty()) 
			value += traits.get("value", type, 1.0, 1000.0) * c;
	}
	if (o->has("alt-mod")) {
		const Object *mod = o->get("alt-mod");
		int c = mod->getCount();
		const std::string& type = mod->getType();
		if (c > 0 && !type.empty()) 
			value += traits.get("value", type, 1.0, 1000.0) * c;
	}
	return value;
}
