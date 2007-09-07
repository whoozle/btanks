
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
#include "special_zone.h"
#include "player_manager.h"
#include "player_slot.h"
#include "net/protocol.h"
#include "game_monitor.h"
#include "player_manager.h"
#include "config.h"
#include "i18n.h"
#include "menu/tooltip.h"
#include "object.h"
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
	}
	
	if (allowed_types.find(type) == allowed_types.end()) 
		throw_ex(("unhanled type '%s'", type.c_str()));	

	_global = type == "timer-lose" || type == "timer-win" || type == "reset-timer" || 
		type == "disable-ai" || type == "enable-ai" || 
		type == "play-tune" || type == "reset-tune";
	
	_final = type == "checkpoint" && name == "final";
	_live =  type == "z-warp";
}

void SpecialZone::onTimer(const int slot_id, const bool win) {
	float duration = (float)atof(subname.c_str());
	LOG_DEBUG(("activating timer %s for %g seconds", name.c_str(), duration));
	PlayerSlot &slot = PlayerManager->getSlot(slot_id);

	std::string key_name = "timer." + name + ".spawn-limit";
	if (Config->has(key_name))
		Config->get(key_name, slot.spawn_limit, 1);
	
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
		onMessage(slot_id);
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
	} else
		throw_ex(("unhandled enter for type '%s'", type.c_str()));
}

void SpecialZone::onMessage(const int slot_id) {
	GameMonitor->displayMessage(area, name, 3, global());
}

void SpecialZone::onHint(const int slot_id) {
	PlayerSlot &slot = PlayerManager->getSlot(slot_id);

	const std::string text = I18n->get(area, name);
	
	Tooltip *tooltip = new Tooltip(text, true);
	slot.tooltips.push(PlayerSlot::Tooltips::value_type(tooltip->getReadingTime(), tooltip));
	//Game->pause();
}

void SpecialZone::onCheckpoint(const int slot_id) {
	if (PlayerManager->isClient())
		return; //no checkpoints on client
	
	PlayerSlot &slot = PlayerManager->getSlot(slot_id);
	{
		int players = PlayerManager->getSlotsCount();
	
		int yn = (int) sqrt((double)size.y * players / size.x);
		if (yn < 1) 
			yn = 1;
	
		int xn = (players - 1) / yn + 1;
		int n = xn * yn;

		int ysize = size.y / yn;
		int xsize = size.x / xn;

		LOG_DEBUG(("position in checkpoint: %d %d of %d[%dx%d]. cell size: %dx%d.", slot_id % xn, slot_id / xn, n, xn, yn, xsize, ysize));

		slot.position.x = position.x + xsize * (slot_id % xn) + xsize / 2;
		slot.position.y = position.y + ysize * (slot_id / xn) + ysize / 2;
		slot.position.z = position.z;
	}
					
	//v3<int> spawn_pos(_zones[c].position + checkpoint_size.convert2v3(0) / 2);
	//slot.position = spawn_pos;
	if (slot.visible) {
		if (final()) {
			Object *o = slot.getObject();
			if (o != NULL) {
				o->addEffect("invulnerability", -1);
			}

			GameMonitor->gameOver("messages", "mission-accomplished", 5, true);
		} else {
			std::string type;
			if (Config->has("multiplayer.game-type"))
				Config->get("multiplayer.game-type", type, "deathmatch");

			if (type != "racing")
				GameMonitor->displayMessage("messages", "checkpoint-reached", 3, false);
		}
	}

	slot.need_sync = true;

	if (slot.remote != -1 && PlayerManager->isServer() ) {
		Message m(Message::TextMessage);
		m.channel = slot_id;
		m.set("area", "messages");
		m.set("message", "checkpoint-reached");
		m.set("duration", "3");
		PlayerManager->send(slot, m);
	}
}

void SpecialZone::onWarp(const int slot_id, const bool enter) {
	PlayerSlot &slot = PlayerManager->getSlot(slot_id);
	Object *o = slot.getObject();
	if (o == NULL)
		return;
}

void SpecialZone::onTick(const int slot_id) {
	PlayerSlot &slot = PlayerManager->getSlot(slot_id);
	Object *o = slot.getObject();
	if (o == NULL)
		return;

	v2<float> pos, vel;
	o->getInfo(pos, vel);
	
	v2<int> left_pos, right_pos; 
	o->getPosition(left_pos);
	o->getPosition(right_pos);
	right_pos += o->size.convert<int>();

	v2<int> c_pos(position.x, position.y);
	c_pos += size / 2;

	int o_z = getBox(o->getZ());
	//LOG_DEBUG(("zone zbox: %d, object zbox: %d", position.z, o_z));
	if (name == "right") {
		if (right_pos.x >= c_pos.x && o_z != (position.z + 1) && vel.x > 0)
			o->setZBox((position.z + 1) * 2000);
		if (right_pos.x < c_pos.x && o_z != position.z && vel.x < 0) 
			o->setZBox(position.z * 2000);
	} else if (name == "left") {
		if (left_pos.x < c_pos.x && o_z != (position.z + 1) && vel.x < 0)
			o->setZBox((position.z + 1) * 2000);
		if (left_pos.x >= c_pos.x && o_z != position.z && vel.x > 0) 
			o->setZBox(position.z * 2000);
	}

	//LOG_DEBUG(("delta left: %d, %d, delta right: %d, %d", left_pos.x - c_pos.x, left_pos.y - c_pos.y, right_pos.x - c_pos.x, right_pos.y - c_pos.y));
}
