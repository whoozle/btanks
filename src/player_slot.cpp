
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
#include "player_slot.h"
#include "world.h"
#include "controls/control_method.h"
#include "menu/tooltip.h"
#include "tmx/map.h"
#include "special_owners.h"
#include "game_monitor.h"
#include "config.h"
#include "object.h"
#include "math/unary.h"
#include "math/binary.h"

#include "i18n.h"

PlayerSlot::PlayerSlot() : 
id(-1), control_method(NULL), need_sync(false), dont_interpolate(false), remote(-1), visible(false), 
classname(), animation(), frags(0), spawn_limit(0), score(0), last_tooltip(NULL)
{}

PlayerSlot::PlayerSlot(const int id) : 
id(id), control_method(NULL), need_sync(false), dont_interpolate(false), remote(-1), visible(false), 
classname(), animation(), frags(0), spawn_limit(0), score(0), last_tooltip(NULL)
{}

void PlayerSlot::serialize(mrt::Serializator &s) const {
	s.add(id);
	//ControlMethod * control_method;
	position.serialize(s);
	s.add(frags);		
	s.add(classname);
	s.add(animation);
	s.add(score);
	s.add(name);
}

void PlayerSlot::deserialize(const mrt::Serializator &s) {
	s.get(id);
	position.deserialize(s);
	s.get(frags);		
	s.get(classname);
	s.get(animation);
	s.get(score);
	s.get(name);
}

Object * PlayerSlot::getObject() {
	if (id < 0) 
		return NULL;
	Object *o = World->getObjectByID(id);
	return o;
}

const Object * PlayerSlot::getObject() const {
	if (id < 0) 
		return NULL;
	const Object *o = World->getObjectByID(id);
	return o;
}


void PlayerSlot::clear() {
	id = -1;
	if (control_method != NULL) {
		delete control_method; 
		control_method = NULL;
	}
	animation.clear();
	classname.clear();

	need_sync = false;
	remote = -1;
	frags = 0;
	net_stats.clear();
	
	zones_reached.clear();
	spawn_limit = 0;
	score = 0;
	name.clear();
	
	while(!tooltips.empty()) {
		delete tooltips.front().second;
		tooltips.pop();
	}
	delete last_tooltip;
	last_tooltip = NULL;
}

void PlayerSlot::displayLast() {
	if (remote != -1)
		return;
	if (tooltips.empty() && last_tooltip != NULL) {
		tooltips.push(Tooltips::value_type(last_tooltip->getReadingTime(), last_tooltip));
		last_tooltip = NULL;
	} else if (!tooltips.empty()) {
		delete last_tooltip;
		last_tooltip = tooltips.front().second;
		tooltips.pop();
	}
}

void PlayerSlot::tick(const float dt) {
	if (remote == -1 && !tooltips.empty()) {
		tooltips.front().first -= dt;
		if (tooltips.front().first < 0) {
			delete last_tooltip;
			last_tooltip = tooltips.front().second;
			tooltips.pop();
		}
	}
	if (!visible) 
		return;
		
	const Object * p = getObject();
	if (p == NULL)
		return;
					
	v2<float> pos, vel;
	p->getInfo(pos, vel);
	vel.normalize();
		
	float moving, idle;
	p->getTimes(moving, idle);
	//vel.fromDirection(p->getDirection(), p->getDirectionsNumber());

	
	moving /= 2;
	if (moving >= 1)
		moving = 1;
	
	GET_CONFIG_VALUE("player.controls.immediate-camera-sliding", bool, ics, false);
	
	map_dst = ics?pos:pos + map_dpos.convert<float>();
	map_dst.x -= viewport.w / 2;
	map_dst.y -= viewport.h / 2;
	validatePosition(map_dst);
		
	//float look_forward = v2<float>(slot.viewport.w, slot.viewport.h, 0).length() / 4;
	//slot.map_dst += vel * moving * look_forward; 

	map_dst_vel = Map->distance(map_dst_pos, map_dst);

	//	if (slot.map_dst_vel.length() > max_speed * 4)
	//		slot.map_dst_vel.normalize(max_speed * 4);
	map_dst_pos += map_dst_vel * math::min<float>(math::abs(dt * 30), 1.0f) * math::sign(dt);
	validatePosition(map_dst_pos);

	//const float max_speed = 2.5 * p->speed;
		
	v2<float> dvel = Map->distance(map_pos, map_dst_pos);

	//const int gran = 50;
	//map_vel = (dvel / (gran / 8)).convert<int>().convert<float>() * gran;
	
	//if (dvel.length() > p->speed) 
	//	dvel.normalize(p->speed);
	map_vel = dvel;
		
	//if (map_vel.length() > max_speed)
	//	map_vel.normalize(max_speed);
		
	map_pos += map_vel * math::min<float>(math::abs(10 * dt), 1) * math::sign(dt);
	//map_pos = map_dst_pos;
	validatePosition(map_pos);
	//LOG_DEBUG(("pos: %g,%g, dst: %g,%g, vel: %g,%g", map_pos.x, map_pos.y, map_dst.x, map_dst.y, map_vel.x, map_vel.y));
}

PlayerSlot::~PlayerSlot() {
	clear();
}


#include "controls/keyplayer.h"
#include "controls/joyplayer.h"
#include "controls/mouse_control.h"
//#include "controls/external_control.h"

void PlayerSlot::createControlMethod(const std::string &control_method_name) {
	delete control_method;
	control_method = NULL;

	if (control_method_name == "keys" || control_method_name == "keys-1" || control_method_name == "keys-2") {
		control_method = new KeyPlayer(control_method_name);
	} else if (control_method_name == "mouse") {
		throw_ex(("fix mouse control method, then disable this exception ;)"));
		control_method = new MouseControl();
	} else if (control_method_name == "joy-1") {
		TRY {
			control_method = new JoyPlayer(0);
			control_method->probe();
		} CATCH("probing control method", {
			delete control_method;
			control_method = new KeyPlayer("keys");
		})
	} else if (control_method_name == "joy-2") {
		TRY {
			control_method = new JoyPlayer(1);
			control_method->probe();
		} CATCH("probing control method", {
			delete control_method;
			control_method = new KeyPlayer("keys");
		})
	} else if (control_method_name != "ai") {
		throw_ex(("unknown control method '%s' used", control_method_name.c_str()));
	}
}

#include "resource_manager.h"
#include "object.h"
#include "config.h"
#include "player_manager.h"
#include "campaign.h"

void PlayerSlot::spawnPlayer(const std::string &classname, const std::string &animation) {
	if (spawn_limit <= 0 && Config->has("map.spawn-limit")) {
		Config->get("map.spawn-limit", spawn_limit, 0);
		const Campaign * campaign = GameMonitor->getCampaign();
		if (campaign != NULL && Config->has("campaign." + campaign->name + ".wares.additional-life.amount")) {
			int al;
			Config->get("campaign." + campaign->name + ".wares.additional-life.amount", al, 0);
			spawn_limit += al;
		}
	}

	Object *obj = ResourceManager->createObject(classname + "(player)", animation);
	assert(obj != NULL);
	
	if (control_method != NULL || remote != -1)
		obj->disable_ai = true;

	obj->setZBox(position.z);
	World->addObject(obj, v2<float>(position.x, position.y) - obj->size / 2, id);

	GET_CONFIG_VALUE("engine.spawn-invulnerability-duration", float, sid, 3);
	obj->addEffect("invulnerability", sid);

	id = obj->getID();
	this->classname = classname;
	this->animation = animation;
	
	std::string type;
	Config->get("multiplayer.game-type", type, "deathmatch");

	if (type == "deathmatch") {
		//moo	
	} else if (type == "racing") {
		Variants v; 
		v.add("racing");
		obj->updateVariants(v);
	} else if (type == "cooperative") {
		/*
		LOG_DEBUG(("prepending cooperative owners."));
		int i, n = PlayerManager->getSlotsCount();
		for(i = 0; i < n; ++i) {
			PlayerSlot &other_slot = PlayerManager->getSlot(i);
			if (other_slot.id == -1 || id == other_slot.id) 
				continue;
			Object *o1 = getObject(), *o2 = other_slot.getObject();
			if (o1 == NULL || o2 == NULL)
				continue;
			o1->prependOwner(other_slot.id);
			o2->prependOwner(id);
		}
		*/
		obj->prependOwner(OWNER_COOPERATIVE);
	} else throw_ex(("unknown multiplayer type '%s' used", type.c_str()));
	
	GameMonitor->addBonuses(*this);
}

void PlayerSlot::validatePosition(v2<float>& position) {
	const v2<int> world_size = Map->getSize();
	if (Map->torus()) {
		if (position.x < 0)
			position.x += world_size.x;
		if (position.y < 0)
			position.y += world_size.y;
		
		if (position.x >= world_size.x)
			position.x -= world_size.x;
		if (position.y >= world_size.y)
			position.y -= world_size.y;
		return;
	}
	
	if (position.x < 0) 
			position.x = 0;
	if (position.x + viewport.w > world_size.x) 
		position.x = world_size.x - viewport.w;

	if (position.y < 0) 
		position.y = 0;
	if (position.y + viewport.h > world_size.y) 
		position.y = world_size.y - viewport.h;
	
	//LOG_DEBUG(("%f %f", mapx, mapy));
}

void PlayerSlot::addScore(const int s) {
	score += s;
	if (score < 0) 
		score = 0;
}
