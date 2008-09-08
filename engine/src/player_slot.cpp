
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
#include "player_slot.h"
#include "world.h"
#include "controls/control_method.h"
#include "menu/tooltip.h"
#include "menu/join_team.h"
#include "tmx/map.h"
#include "i18n.h"
#include "special_owners.h"
#include "game_monitor.h"
#include "config.h"
#include "object.h"
#include "math/unary.h"
#include "math/binary.h"
#include "mrt/random.h"
#include "player_manager.h"
#include "rt_config.h"

#include "i18n.h"

PlayerSlot::PlayerSlot() : 
id(-1), control_method(NULL), need_sync(false), dont_interpolate(false), remote(-1), visible(false), 
classname(), animation(), frags(0), spawn_limit(0), dead_time(0), score(0), spectator(false), team(Team::None), 
last_tooltip(NULL), last_tooltip_used(false), join_team(NULL) , moving(0)
{}

void PlayerSlot::serialize(mrt::Serializator &s) const {
	s.add(id);
	//ControlMethod * control_method;
	s.add(position);
	s.add(frags);		
	s.add(classname);
	s.add(animation);
	s.add(score);
	s.add(spawn_limit);
	s.add(name);
	s.add(spectator);
	s.add((int)team);
}

void PlayerSlot::deserialize(const mrt::Serializator &s) {
	s.get(id);
	s.get(position);
	s.get(frags);		
	s.get(classname);
	s.get(animation);
	s.get(score);
	s.get(spawn_limit);
	s.get(name);
	s.get(spectator);
	int t;
	s.get(t);
	team = (Team::ID)t;
}

Object * PlayerSlot::getObject() const {
	if (id < 0) 
		return NULL;
	return World->getObjectByID(id);
}

void PlayerSlot::clear() {
	id = -1;
	if (control_method != NULL) {
		delete control_method; 
		control_method = NULL;
	}
	old_state.clear();
	animation.clear();
	classname.clear();

	need_sync = false;
	remote = -1;
	frags = 0;
	net_stats.clear();
	
	zones_reached.clear();
	spawn_limit = 0;
	dead_time = 0;
	score = 0;
	name.clear();
	spectator = false;
	team = Team::None;
	
	while(!tooltips.empty()) {
		delete tooltips.front().second;
		tooltips.pop();
	}
	delete last_tooltip;
	last_tooltip = NULL;
	last_tooltip_used = false;
	delete join_team;
	join_team = NULL;
	moving = 0;
}

void PlayerSlot::displayLast() {
	if (remote != -1)
		return;
	if (tooltips.empty() && last_tooltip != NULL) {
		tooltips.push(Tooltips::value_type(last_tooltip->getReadingTime(), last_tooltip));
		last_tooltip = NULL;
		last_tooltip_used = true;
	} else if (!tooltips.empty()) {
		delete last_tooltip;
		last_tooltip = tooltips.front().second;
		if (!last_tooltip_used)
			GameMonitor->onTooltip("hide", PlayerManager->get_slot_id(id), last_tooltip->area, last_tooltip->message);
		last_tooltip_used = false;
		tooltips.pop();
		if (!tooltips.empty()) {
			GameMonitor->onTooltip("show", PlayerManager->get_slot_id(id), tooltips.front().second->area, tooltips.front().second->message);	
		}
	}
}

void PlayerSlot::removeTooltips() {
	if (remote != -1)
		return;
	
	while(!tooltips.empty()) {
		delete last_tooltip;
		last_tooltip = tooltips.front().second;
		if (!last_tooltip_used)
			GameMonitor->onTooltip("hide", PlayerManager->get_slot_id(id), last_tooltip->area, last_tooltip->message);		
		last_tooltip_used = false;
		tooltips.pop();
	} 
}

#include "player_manager.h"

void PlayerSlot::displayTooltip(const std::string &area, const std::string &message) {
	Tooltip *tooltip = new Tooltip(area, message, true);
	if (tooltips.empty()) {
		GameMonitor->onTooltip("show", PlayerManager->get_slot_id(id), area, message);	
	}
	tooltips.push(PlayerSlot::Tooltips::value_type(tooltip->getReadingTime(), tooltip));
}

void PlayerSlot::tick(const float dt) {
	if (!tooltips.empty()) {
		tooltips.front().first -= dt;
		if (tooltips.front().first < 0) {
			delete last_tooltip;
			last_tooltip = tooltips.front().second;
	
			if (!last_tooltip_used)
				GameMonitor->onTooltip("hide", PlayerManager->get_slot_id(id), last_tooltip->area, last_tooltip->message);
			last_tooltip_used = false;
			
			tooltips.pop();
			if (!tooltips.empty()) {
				GameMonitor->onTooltip("show", PlayerManager->get_slot_id(id), tooltips.front().second->area, tooltips.front().second->message);	
			}
		}
	}
	if (!visible) 
		return;

	if (RTConfig->game_type == GameTypeCTF || RTConfig->game_type == GameTypeTeamDeathMatch) {
		if (team == Team::None) {
			if (join_team == NULL)
				join_team = new JoinTeamControl;
			join_team->tick(dt);			
		} else {
			delete join_team;
			join_team = NULL;		
		}
	}
		
	const Object * p = getObject();
	if (p == NULL) {
		moving = 0;
		return;
	}
					
	v2<float> pos, vel;
	p->get_position(pos);
	p->get_velocity(vel);
	vel.normalize();
		
	//vel.fromDirection(p->get_direction(), p->get_directions_number());
	if (!vel.is0())
		moving += dt;
	
	if (moving >= 2)
		moving = 2;
	
	GET_CONFIG_VALUE("player.controls.immediate-camera-sliding", bool, ics, false);
	
	map_dst = ics?pos:pos + map_dpos.convert<float>();
	map_dst.x -= viewport.w / 2;
	map_dst.y -= viewport.h / 2;
	validatePosition(map_dst);
		
	//float look_forward = v2<float>(slot.viewport.w, slot.viewport.h, 0).length() / 4;
	//slot.map_dst += vel * moving / 2 * look_forward; 

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

void PlayerSlot::join(const Team::ID t) {
	team = t;
	spectator = false;
	delete join_team;
	join_team = NULL;
	
	std::string v, a;
	getDefaultVehicle(v, a); //hack to recreate animation 
	classname = v;
	animation = a;	
}

void PlayerSlot::updateState(PlayerState &state, const float dt) {
	if (control_method == NULL)
		throw_ex(("updateState called on slot without control_method"));
	//handle custom stuff here. 
	if (join_team != NULL && team == Team::None) {
		PlayerState s;
		s = old_state;		
		control_method->updateState(*this, state, dt);
		if (state.left && !s.left) {
			//LOG_DEBUG(("left"));
			join_team->left();
		} 
		if (state.right && !s.right) {
			//LOG_DEBUG(("right"));
			join_team->right();
		}
		join_team->reset(); 
		
		if (state.fire && !s.fire) {
			int t = join_team->get();
			if (t < 0 || t >= 4) 
				throw_ex(("invalid team %d", t));
			
			LOG_DEBUG(("choosing team %d", t));
			join((Team::ID)t);
		}
	} else {
		control_method->updateState(*this, state, dt);
	}
}


void PlayerSlot::createControlMethod(const std::string &control_method_name) {
	delete control_method;
	control_method = NULL;

	if (control_method_name == "keys" || control_method_name == "keys-1" || control_method_name == "keys-2") {
		control_method = new KeyPlayer(control_method_name);
	} else if (control_method_name == "mouse") {
		//throw_ex(("fix mouse control method, then disable this exception ;)"));
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

void PlayerSlot::spawn_player(const int slot_id, const std::string &classname_, const std::string &animation_) {
	classname = classname_;
	animation = animation_;
	
	if ((RTConfig->game_type == GameTypeTeamDeathMatch || RTConfig->game_type == GameTypeCTF) && team == Team::None) {
		if (control_method != NULL || remote >= 0) {
			LOG_DEBUG(("team mode on, do not respawn player %d before (s)he joins any team", slot_id));
			id = 0; //hack hack hack! :)
			spectator = true;
			return;
		} else if (remote == -1) {
			//assigning AIs to teams automatically !!
			int n = PlayerManager->get_slots_count();
			int stats[4] = {0, 0, 0, 0};
			for(int i = 0; i < n; ++i) {
				PlayerSlot &slot = PlayerManager->get_slot(i);
				if (slot.team != Team::None) 
					++stats[(int)slot.team];
			};
			
			Team::ID team = Team::Red;
			int min = stats[0];
			
			for(int i = 1; i < RTConfig->teams; ++i) {
				if (stats[i] < min) {
					team = (Team::ID)i;
					min = stats[i];
				}
			}
			
			LOG_DEBUG(("auto assign ai slot %d: %s", slot_id, Team::get_color(team)));
			join(team);
		}
	}

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
	obj->set_slot(slot_id);
	
	if (control_method != NULL || remote != -1)
		obj->disable_ai = true;

	obj->set_zbox(position.z);
	
	bool random_respawn = false;
	GameType game_type = RTConfig->game_type;
	if (game_type == GameTypeDeathMatch) {
		Config->get("multiplayer.random-respawn", random_respawn, false);
	}
	
	if (game_type == GameTypeCTF || random_respawn) {
		//const Matrix<int>& matrix = Map->get_impassability_matrix(ZBox::getBox(position.z));
		Matrix<int> matrix;
		World->get_impassability_matrix(matrix, obj, NULL);
		
		const v2<int> tile_size = Map->getPathTileSize();
		if (obj->size.is0())
			throw_ex(("object size must not be 0,0"));
		
		v2<int> obj_size = ((obj->size.convert<int>() - 1) / tile_size) + 1;
		LOG_DEBUG(("searching random %dx%d spot", obj_size.x, obj_size.y));
	
		int w = matrix.get_width(), h = matrix.get_height();
		std::vector<v2<int> > spots;
		for(int y = 0; y < h - obj_size.y + 1; ++y) 
			for(int x= 0; x < w - obj_size.x + 1; ++x) {
				for(int yy = 0; yy < obj_size.y; ++yy)
					for(int xx = 0; xx < obj_size.x; ++xx) {
						int im = matrix.get(y + yy, x + xx);
						if (im < 0 || im >= 100)
							goto skip;
					
					}
				spots.push_back(v2<int>(x, y));
			skip: ;
			}
		
		size_t n = spots.size();
		if (n == 0)
			throw_ex(("no spots found"));
		
		int idx = mrt::random(n);
		LOG_DEBUG(("found %u spots. get #%d", (unsigned)n, idx));
		v2<float> pos = (spots[idx] * tile_size).convert<float>();
		
		//LOG_DEBUG(("map : %s", matrix.dump().c_str()));
		obj_size = tile_size * obj_size / 2;
		World->addObject(obj, pos + obj_size.convert<float>() - obj->size / 2, id);
	} else {
		World->addObject(obj, v2<float>(position.x, position.y) - obj->size / 2, id);
	}
	id = obj->get_id();

	GET_CONFIG_VALUE("engine.spawn-invulnerability-duration", float, sid, 3);
	obj->add_effect("invulnerability", sid);

	switch(game_type) {
		case GameTypeDeathMatch: 
			break;

		case GameTypeTeamDeathMatch: 
		case GameTypeCTF: 
			if (team != Team::None)
				obj->prepend_owner(Team::get_owner(team));
			break;

		case GameTypeRacing: {
			Variants v; 
			v.add("racing");
			obj->update_variants(v);
			break;
		}

		case GameTypeCooperative: 
			obj->prepend_owner(OWNER_COOPERATIVE);
			break;

		default:
			throw_ex(("unknown multiplayer type %d used", (int)game_type));
	}
	
	GameMonitor->addBonuses(*this);
	dead_time = 0;
}

void PlayerSlot::validatePosition(v2<float>& position) {
	const v2<int> world_size = Map->get_size();
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
	
	if (viewport.w >= world_size.x) {
		position.x = (world_size.x - viewport.w) / 2;
	} else {
		if (position.x < 0) 
			position.x = 0;
		if (position.x + viewport.w > world_size.x) 
			position.x = world_size.x - viewport.w;
	}

	if (viewport.h >= world_size.y) {
		position.y = (world_size.y - viewport.h) / 2;
	} else {
		if (position.y < 0) 
			position.y = 0;
		if (position.y + viewport.h > world_size.y) 
			position.y = world_size.y - viewport.h;
	}
	
	//LOG_DEBUG(("%f %f", mapx, mapy));
}

void PlayerSlot::addScore(const int s) {
	score += s;
	if (score < 0) 
		score = 0;
}

void PlayerSlot::setViewport(const sdlx::Rect &rect) {
	visible = true;
	viewport = rect;
	const Object *o = getObject();
	if (o == NULL)
		return;
	
	v2<float> pos = o->get_center_position();
	map_pos.x = (int)pos.x - rect.w / 2;
	map_pos.y = (int)pos.y - rect.h / 2;
}

void PlayerSlot::getDefaultVehicle(std::string &vehicle, std::string &animation) {
	std::string rv, ra;
	Config->get("multiplayer.restrict-start-vehicle", rv, std::string());
	Config->get("multiplayer.restrict-start-animation", ra, std::string());

	if (!this->classname.empty()) {
		vehicle = this->classname;
	} else if (rv.empty()) {
		if (vehicle.empty()) 
			Config->get("menu.default-vehicle-1", vehicle, "tank");
	} else vehicle = rv;

	static const char * colors[4] = {"red", "green", "blue", "yellow"};
	if (team != Team::None && (vehicle == "tank" || vehicle == "launcher" || vehicle == "shilka")) {
		LOG_DEBUG(("picking team color %d", (int)team));
		animation = colors[(int)team];
		animation += "-" + vehicle;
	} else if (!this->animation.empty()) {
		animation = this->animation;
	} else if (ra.empty()) {
		if (animation.empty()) {
			if (vehicle == "tank" || vehicle == "launcher" || vehicle == "shilka") {
				animation = colors[mrt::random(4)];
				animation += "-" + vehicle;
			} else animation = vehicle;
		}
	} else animation = ra;
}

#include "sdlx/cursor.h"

void PlayerSlot::render(sdlx::Surface &window, const int vx, const int vy) {
	viewport.x += vx;
	viewport.y += vy;

	GET_CONFIG_VALUE("player.controls.immediate-camera-sliding", bool, ics, false);		
			
	v2<float> pos = ics?map_pos + map_dpos.convert<float>() : map_pos;
	validatePosition(pos);
			
	World->render(window, sdlx::Rect((int)pos.x, (int)pos.y, viewport.w, viewport.h), 
		viewport, -10000, 10001, getObject());

	const Tooltip *t = currentTooltip();
	if (t != NULL) {
		int w, h;
		t->get_size(w, h);
		t->render(window, viewport.x, viewport.h - h);
	}

	viewport.x -= vx;
	viewport.y -= vy;

	if (join_team != NULL && team == Team::None) {
		int w, h;
		join_team->get_size(w, h);
		join_team->render(window, viewport.x + (viewport.w - w) / 2, viewport.y + (viewport.h - h) / 2);
	}
}
