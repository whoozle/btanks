
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

/* 
 * Additional rights can be granted beyond the GNU General Public License 
 * on the terms provided in the Exception. If you modify this file, 
 * you may extend this exception to your version of the file, 
 * but you are not obligated to do so. If you do not wish to provide this
 * exception without modification, you must delete this exception statement
 * from your version and license this file solely under the GPL without exception. 
*/
#include "special_zone.h"
#include "player_manager.h"
#include "player_slot.h"
#include "net/message.h"
#include "game_monitor.h"
#include "player_manager.h"
#include "config.h"
#include "menu/tooltip.h"
#include "object.h"
#include "rt_config.h"
#include "sound/mixer.h"
#include <set>

SpecialZone::~SpecialZone() {}

SpecialZone::SpecialZone(const ZBox & zbox, const std::string &type, const std::string &name, const std::string &subname) :
	 ZBox(zbox), type(type), name(name), subname(subname) {
	static std::set<std::string> allowed_types; 
	if (allowed_types.empty()) {
		allowed_types.insert("checkpoint");
		allowed_types.insert("hint");
		allowed_types.insert("message");
		allowed_types.insert("timer-lose");
		allowed_types.insert("timer-win");
		allowed_types.insert("reset-timer");
		allowed_types.insert("disable-ai");
		allowed_types.insert("enable-ai");
		allowed_types.insert("play-tune");
		allowed_types.insert("reset-tune");
		allowed_types.insert("z-warp");
		allowed_types.insert("script");
		allowed_types.insert("local-script");
	}
	
	if (allowed_types.find(type) == allowed_types.end()) 
		throw_ex(("unhanled type '%s'", type.c_str()));	

	_global = type == "timer-lose" || type == "timer-win" || type == "reset-timer" || 
		type == "disable-ai" || type == "enable-ai" || 
		type == "play-tune" || type == "reset-tune" || type == "script";
	
	_final = type == "checkpoint" && name == "final";
	_live =  type == "z-warp";
}

void SpecialZone::onTimer(const int slot_id, const bool win) {
	float duration = (float)atof(subname.c_str());
	LOG_DEBUG(("activating timer %s for %g seconds", name.c_str(), duration));

	int spawn_limit = 0;
	std::string key_name = "timer." + name + ".spawn-limit";
	if (Config->has(key_name))
		Config->get(key_name, spawn_limit, 1);
	
	if (spawn_limit > 0) 
		for(size_t i = 0; i < PlayerManager->get_slots_count(); ++i) {
			PlayerSlot &slot = PlayerManager->get_slot(i);
			slot.spawn_limit = spawn_limit;
		}
	
	if (win) {
		GameMonitor->setTimer("messages", "mission-accomplished", duration, true);
	} else 
		GameMonitor->setTimer("messages", "game-over", duration, false);
		
	GameMonitor->displayMessage(area, name, 3, global());
}

void SpecialZone::onExit(const int slot_id) {
	if (type == "z-warp") {
		onWarp(slot_id, false);
	} else if (_live) 
		throw_ex(("unhandled exit for type '%s'", type.c_str()));
}

void SpecialZone::onEnter(const int slot_id) {
	if (type == "checkpoint") 
		onCheckpoint(slot_id);
	else if (type == "hint") 
		onHint(slot_id);
	else if (type == "message") 
		on_message(slot_id);
	else if (type == "timer-lose") 
		onTimer(slot_id, false);
	else if (type == "timer-win") 
		onTimer(slot_id, true);
	else if (type == "reset-timer") 
		GameMonitor->resetTimer();
	else if (type == "disable-ai") 
		GameMonitor->disable(name);
	else if (type == "enable-ai") 
		GameMonitor->disable(name, false);
	else if (type == "play-tune") 
		Mixer->play(name, true);
	else if (type == "reset-tune") 
		Mixer->reset();
	else if (type == "z-warp") {
		onWarp(slot_id, true);
	} else if (type == "script") {
		GameMonitor->onScriptZone(slot_id, *this, true);
	} else if (type == "local-script") {
		GameMonitor->onScriptZone(slot_id, *this, false);
	} else
		throw_ex(("unhandled enter for type '%s'", type.c_str()));
}

void SpecialZone::on_message(const int slot_id) {
	GameMonitor->displayMessage(area, name, 3, global());
}

void SpecialZone::onHint(const int slot_id) {
	PlayerSlot &slot = PlayerManager->get_slot(slot_id);

	//Game->pause();
	if (slot.remote != -1 && !PlayerManager->is_client()) //useless but just for sure
		PlayerManager->send_hint(slot_id, area, name);
	else 
		slot.displayTooltip(area, name);
}

const v3<int> SpecialZone::getPlayerPosition(const int slot_id) const {
	int players = PlayerManager->get_slots_count();

	int yn = (int) sqrt((double)size.y * players / size.x);
	if (yn < 1) 
		yn = 1;

	int xn = (players - 1) / yn + 1;
	//int n = xn * yn;

	int ysize = size.y / yn;
	int xsize = size.x / xn;

	//LOG_DEBUG(("position in checkpoint: %d %d of %d[%dx%d]. cell size: %dx%d.", slot_id % xn, slot_id / xn, n, xn, yn, xsize, ysize));
	return v3<int>(
		position.x + xsize * (slot_id % xn) + xsize / 2, 
		position.y + ysize * (slot_id / xn) + ysize / 2, 
		position.z
	);
}


void SpecialZone::onCheckpoint(const int slot_id) {
	if (PlayerManager->is_client())
		return; //no checkpoints on client

	GameType game_type = RTConfig->game_type;
	
	PlayerSlot &slot = PlayerManager->get_slot(slot_id);
	slot.need_sync = true;
	
	if (game_type == GameTypeRacing) {
		const SpecialZone &zone = PlayerManager->get_next_checkpoint(slot);
		if (zone.name != name) {
			LOG_DEBUG(("wrong checkpoint, next checkpoint: %s", zone.name.c_str()));
			GameMonitor->displayMessage("messages", "wrong-checkpoint", 3, false);
			return;
		}
		PlayerManager->fix_checkpoints(slot, zone); //remove all wrong checkpoints from list
	}
	slot.position = getPlayerPosition(slot_id);
					
	//v3<int> spawn_pos(_zones[c].position + checkpoint_size.convert2v3(0) / 2);
	//slot.position = spawn_pos;
	if (final()) {
		GameMonitor->game_over("messages", "mission-accomplished", 5, true);
		return;
	}
	
	if (slot.visible) {
		if (game_type != GameTypeRacing)
			GameMonitor->displayMessage("messages", "checkpoint-reached", 3, false);
	} else {
		if (slot.remote != -1 && PlayerManager->is_server() ) {
			Message m(Message::TextMessage);
			m.channel = slot_id;
			m.set("hint", "0");
			m.set("area", "messages");
			m.set("message", "checkpoint-reached");
			m.set("duration", "3");
			PlayerManager->send(slot, m);
		}
	}
}

void SpecialZone::onWarp(const int slot_id, const bool enter) {
	PlayerSlot &slot = PlayerManager->get_slot(slot_id);
	Object *o = slot.getObject();
	if (o == NULL)
		return;
}

void SpecialZone::onTick(const int slot_id) {
	PlayerSlot &slot = PlayerManager->get_slot(slot_id);
	Object *o = slot.getObject();
	if (o == NULL)
		return;

	v2<float> pos, vel;
	o->get_position(pos); o->get_velocity(vel);
	
	
	v2<int> left_pos, right_pos; 
	o->get_position(left_pos);
	o->get_position(right_pos);
	right_pos += o->size.convert<int>();

	v2<int> c_pos(position.x, position.y);
	c_pos += size / 2;

	int o_z = getBox(o->get_z());
	//LOG_DEBUG(("zone zbox: %d, object zbox: %d", position.z, o_z));
	if (name == "right") {
		if (right_pos.x >= c_pos.x && o_z != (position.z + 1) && vel.x > 0)
			o->set_zbox((position.z + 1) * 2000);
		if (right_pos.x < c_pos.x && o_z != position.z && vel.x < 0) 
			o->set_zbox(position.z * 2000);
	} else if (name == "left") {
		if (left_pos.x < c_pos.x && o_z != (position.z + 1) && vel.x < 0)
			o->set_zbox((position.z + 1) * 2000);
		if (left_pos.x >= c_pos.x && o_z != position.z && vel.x > 0) 
			o->set_zbox(position.z * 2000);
	}

	//LOG_DEBUG(("delta left: %d, %d, delta right: %d, %d", left_pos.x - c_pos.x, left_pos.y - c_pos.y, right_pos.x - c_pos.x, right_pos.y - c_pos.y));
}
