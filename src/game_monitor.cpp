
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
#include <string>
#include <stdexcept>

#include "game_monitor.h"
#include "object.h"
#include "config.h"
#include "world.h"
#include "resource_manager.h"
#include "player_manager.h"
#include "game.h"
#include "i18n.h"
#include "sdlx/font.h"
#include "sdlx/surface.h"
#include "special_owners.h"
#include "mrt/random.h"
#include "tmx/map.h"
#include "sound/mixer.h"
#include "window.h"
#include "var.h"
#include "special_zone.h"
#include "math/unary.h"
#include <stdlib.h>
#include "player_slot.h"

IMPLEMENT_SINGLETON(GameMonitor, IGameMonitor);

IGameMonitor::IGameMonitor() : _game_over(false), _check_items(0.5, true), _state_timer(false), _timer(0) {}

void Item::respawn() {
	LOG_DEBUG(("respawning item: %s:%s", classname.c_str(), animation.c_str()));
	Object *o = ResourceManager->createObject(classname, animation);
	if (z) 
		o->setZ(z, true);
	o->addOwner(OWNER_MAP);
	
	World->addObject(o, position.convert<float>());
	id = o->getID();
	dead_on = 0;
}

void Item::updateMapProperty() {
	if (z) 
		Map->properties[property] = mrt::formatString("%d,%d,%d", position.x, position.y, z);
	else 
		Map->properties[property] = mrt::formatString("%d,%d", position.x, position.y);
}

void IGameMonitor::eraseLast(const std::string &property) {
	if (_items.empty())
		throw_ex(("item list is empty!"));
	if (_items.back().property != property)
		throw_ex(("eraseLast: %s is not the latest item in list", property.c_str()));
	_items.pop_back();
}

const Item& IGameMonitor::find(const Object *obj) const {
	for(Items::const_iterator i = _items.begin(); i != _items.end(); ++i) {
		const Item &item = *i;
		Object *o = World->getObjectByID(item.id);
		if (obj == o) 
			return item;
	}
	throw_ex(("could not find item %s:%s", obj->registered_name.c_str(), obj->animation.c_str()));
}

Item& IGameMonitor::find(const Object *obj) {
	for(Items::iterator i = _items.begin(); i != _items.end(); ++i) {
		Object *o = World->getObjectByID(i->id);
		if (obj == o) 
			return *i;
	}
	throw_ex(("could not find item %s:%s", obj->registered_name.c_str(), obj->animation.c_str()));
}

Item& IGameMonitor::find(const std::string &property) {
	for(Items::iterator i = _items.begin(); i != _items.end(); ++i) {
		if (i->property == property) 
			return *i;
	}
	throw_ex(("could not find item %s", property.c_str()));
}

void IGameMonitor::checkItems(const float dt) {	
	if (_game_over || !_check_items.tick(dt))
		return;
		
	int goal = 0, goal_total = 0;
	
	if (!_destroy_classes.empty()) {
		++goal_total;
		if (!World->itemExists(_destroy_classes)) 
			++goal;
	}
	
	_specials.clear();
	
	for(Items::iterator i = _items.begin(); i != _items.end(); ++i) {
		Item &item = *i;
		Object *o = World->getObjectByID(item.id);

		bool dead = true;
		if (o != NULL) {
			dead = o->getState() == "broken";
		}
		
		if (item.destroy_for_victory) {
			++goal_total;
			if (dead) {
				++goal;
			}
		} 

		if (!dead) {
			if (item.destroy_for_victory || !item.save_for_victory.empty()) {
				v2<int> pos;
				o->getCenterPosition(pos);
				_specials.push_back(v3<int>(pos.x, pos.y, o->getID()));	
			}

			continue;
		}
		//object is dead.
		
		if (!item.save_for_victory.empty()) {
			gameOver("messages", item.save_for_victory, 5);
			continue;
		}

		if (o)
			continue;
		
		Uint32 ticks = SDL_GetTicks();
		if (item.dead_on == 0) {
			item.dead_on = ticks;
			LOG_DEBUG(("item %d:%s:%s is dead, log dead time.", item.id, item.classname.c_str(), item.animation.c_str()));
			continue;
		}
		
		int rt;
		Config->get("map." + item.classname + ".respawn-interval", rt, 5); 
		if (rt < 0) 
			continue;
		if (((ticks - item.dead_on) / 1000) >= (unsigned)rt) {
			//respawning item
			item.respawn();
		}
	}
	if (goal_total > 0 && goal == goal_total) {
		gameOver("messages", "mission-accomplished", 5);
	}
}

void IGameMonitor::add(const Item &item_) {
	Item item(item_);
	Object *o = ResourceManager->createObject(item.classname, item.animation);
	if (item.z)
		o->setZ(item.z, true);
	
	o->addOwner(OWNER_MAP);
	World->addObject(o, v2<float>(item.position.x, item.position.y));
	
	if (item.destroy_for_victory || !item.save_for_victory.empty()) {
		LOG_DEBUG(("%s:%s critical for victory", item.classname.c_str(), item.animation.c_str()));
	}
		
	item.id = o->getID();
	_items.push_back(item);
}

void IGameMonitor::pushState(const std::string &state, const float time) {
	_state = state;
	_state_timer.set(time);
}

const std::string IGameMonitor::popState(const float dt) {
	if (_state.empty() || !_state_timer.tick(dt))
		return std::string();
	std::string r = _state;
	_state.clear();
	return r;
}

void IGameMonitor::gameOver(const std::string &area, const std::string &message, const float time) {
	_game_over = true;
	displayMessage(area, message, time);
	PlayerManager->gameOver(message, time);
	resetTimer();
}

void IGameMonitor::displayMessage(const std::string &area, const std::string &message, const float time) {
	pushState(I18n->get(area, message), time);
}

void IGameMonitor::setTimer(const std::string &area, const std::string &message, const float time) {
	_timer_message_area = area;
	_timer_message = message;
	_timer = time;
}

void IGameMonitor::resetTimer() {
	_timer_message.clear();
	_timer = 0;
}

void IGameMonitor::clear() {
	resetTimer();

	_game_over = false;
	_state.clear();
	
	_items.clear();
	_specials.clear();
	_check_items.reset();
	_disabled.clear();
	_destroy_classes.clear();

	_waypoints.clear();
	_all_waypoints.clear();
	_waypoint_edges.clear();
}

void IGameMonitor::tick(const float dt) {	
	const bool client = PlayerManager->isClient();

	if (!_timer_message.empty() && _timer > 0) {
		_timer -= dt;
		if (_timer <= 0) {
			if (!client)
				gameOver(_timer_message_area, _timer_message, 5);
			_timer = 0;
		}
	}

	std::string game_state = popState(dt);
	if (_game_over && !game_state.empty()) {
		if (!_campaign.empty()) {
			PlayerSlot &slot = PlayerManager->getSlot(0); 
			int score; 
			Config->get("campaign." + _campaign + ".score", score, 0);
			score += slot.score;
			Config->set("campaign." + _campaign + ".score", score);
			LOG_DEBUG(("total score: %d", score));
		}
		LOG_DEBUG(("saving compaign state..."));
		
		Game->clear();
	}
}

void IGameMonitor::render(sdlx::Surface &window) {
	static const sdlx::Font * _big_font;
	if (_big_font == NULL)
		_big_font = ResourceManager->loadFont("big", true);

	if (!_state.empty()) {
		int w = _big_font->render(NULL, 0, 0, _state);
		int x = (window.getWidth() - w) / 2;
		int y = (window.getHeight() - _big_font->getHeight()) / 2;
		
		_big_font->render(window, x, y, _state);
	}

	if (_timer > 0) {
		int m = (int)_timer / 60;
		int ms = (int)(10 * (_timer - (int)_timer));
		std::string timer_str; 
		if (m) {
			timer_str = mrt::formatString("%2d%c%02d", m, (ms / 2 == 0 || ms /2 == 1 || ms / 2 == 4)?':':'.', ((int)_timer) % 60);
		} else 
			timer_str = mrt::formatString("   %2d.%d", (int)_timer, ms);
		
		int tw = timer_str.size() + 1;
		_big_font->render(window, window.getWidth() - _big_font->getWidth() * tw, 
			 window.getHeight() - _big_font->getHeight() * 3 / 2, 
			 timer_str);
	}

}


const bool IGameMonitor::disabled(const Object *o) const {
	return _disabled.find(o->classname) != _disabled.end() || _disabled.find(o->registered_name) != _disabled.end();
}

void IGameMonitor::disable(const std::string &classname, const bool value) {
	LOG_DEBUG(("%s ai for classname %s", value?"disabling":"enabling", classname.c_str()));
	if (value) {
		_disabled.insert(classname);
	} else {
		_disabled.erase(classname);
	}
}


#include "mrt/serializator.h"

void IGameMonitor::serialize(mrt::Serializator &s) const {
TRY {
	s.add(_game_over);

	int n = (int)_specials.size();
	s.add(n);
	for(int i = 0; i < n; ++i)	
		s.add(_specials[i]);

	s.add(_state);
	s.add(_state_timer);
	
	s.add(_timer_message);
	s.add(_timer_message_area);
	s.add(_timer);

	n = (int)_disabled.size();
	s.add(n);
	for(std::set<std::string>::const_iterator i = _disabled.begin(); i != _disabled.end(); ++i) {
		s.add(*i);
	}

	n = (int)_destroy_classes.size();
	s.add(n);
	for(std::set<std::string>::const_iterator i = _destroy_classes.begin(); i != _destroy_classes.end(); ++i) {
		s.add(*i);
	}

} CATCH("serialize", throw);
}

void IGameMonitor::deserialize(const mrt::Serializator &s) {
TRY {
	s.get(_game_over);

	int n;
	s.get(n);
	_specials.clear();
	while(n--) {
		v3<int> p;
		s.get(p);
		_specials.push_back(p);
	}

	s.get(_state);
	s.get(_state_timer);
	
	s.get(_timer_message);
	s.get(_timer_message_area);
	s.get(_timer);

	s.get(n);
	_disabled.clear();
	while(n--) {
		std::string d;
		s.get(d);
		_disabled.insert(d);
	}

	s.get(n);
	_destroy_classes.clear();
	while(n--) {
		std::string d;
		s.get(d);
		_destroy_classes.insert(d);
	}
	
} CATCH("deserialize", throw);
}

void IGameMonitor::killAllClasses(const std::set<std::string> &classes) {
	_destroy_classes = classes;
}


const std::string IGameMonitor::getRandomWaypoint(const std::string &classname, const std::string &last_wp) const {
	if (last_wp.empty()) 
		throw_ex(("getRandomWaypoint('%s', '%s') called with empty name", classname.c_str(), last_wp.c_str()));
	
	WaypointClassMap::const_iterator wp_class = _waypoints.find(classname);
	if (wp_class == _waypoints.end()) 
		throw_ex(("no waypoints for '%s' defined", classname.c_str()));
		
	WaypointEdgeMap::const_iterator b = _waypoint_edges.lower_bound(last_wp);
	WaypointEdgeMap::const_iterator e = _waypoint_edges.upper_bound(last_wp);
	if (b == e) 
		throw_ex(("no edges defined for waypoint '%s'", last_wp.c_str()));

	int wp = mrt::random(_waypoint_edges.size() * 2);
	while(true) {
		for(WaypointEdgeMap::const_iterator i = b; i != e; ++i) {
			if (wp-- <= 0) {
				return i->second;
			}
		}
	}
	throw_ex(("getRandomWaypoint(unexpected termination)"));
	return "*bug*";
}

const std::string IGameMonitor::getNearestWaypoint(const BaseObject *obj, const std::string &classname) const {
	v2<int> pos;
	obj->getPosition(pos);
	int distance = -1;
	std::string wp;
	
	WaypointClassMap::const_iterator i = _waypoints.find(classname);
	if (i == _waypoints.end())
		throw_ex(("no waypoints for '%s' found", classname.c_str()));

	for(WaypointMap::const_iterator j = i->second.begin(); j != i->second.end(); ++j) {
		int d = j->second.quick_distance(pos);
		if (distance == -1 || d < distance) {
			distance = d;
			wp = j->first;
		}
	}
	return wp;
}


void IGameMonitor::getWaypoint(v2<float> &wp, const std::string &classname, const std::string &name) {
	if (name.empty() || classname.empty()) 
		throw_ex(("getWaypoint('%s', '%s') called with empty classname and/or name", classname.c_str(), name.c_str()));
	
	WaypointClassMap::const_iterator wp_class = _waypoints.find(classname);
	if (wp_class == _waypoints.end()) 
		throw_ex(("no waypoints for '%s' defined", classname.c_str()));
	
	WaypointMap::const_iterator i = wp_class->second.find(name);
	if (i == wp_class->second.end())
		throw_ex(("no waypoints '%s' defined", name.c_str()));
	wp = i->second.convert<float>();
}

void IGameMonitor::renderWaypoints(sdlx::Surface &surface, const sdlx::Rect &src, const sdlx::Rect &dst) {
	const sdlx::Surface *s = ResourceManager->loadSurface("car-waypoint.png");
	
	for(WaypointClassMap::const_iterator i = _waypoints.begin(); i != _waypoints.end(); ++i) {
		//const std::string &classname = i->first;
		for(WaypointMap::const_iterator j = i->second.begin(); j != i->second.end(); ++j) {
			const v2<int> &wp = j->second;
			surface.copyFrom(*s, 
			wp.x - src.x + dst.x, 
			wp.y - src.y + dst.y - s->getHeight());	
		}
	}
	
	s = ResourceManager->loadSurface("edge.png");
	int w = s->getWidth() / 3, h = s->getHeight();
	sdlx::Rect normal(0, 0, w, h), out(w, 0, w, h), in(2 * w, 0, w, h);
	
	for(WaypointEdgeMap::const_iterator i = _waypoint_edges.begin(); i != _waypoint_edges.end(); ++i) {
		WaypointMap::const_iterator a = _all_waypoints.find(i->first);
		if (a == _all_waypoints.end()) 
			throw_ex(("no waypoint '%s' defined", i->first.c_str()));
		WaypointMap::const_iterator b = _all_waypoints.find(i->second);
		if (b == _all_waypoints.end()) 
			throw_ex(("no waypoint '%s' defined", i->second.c_str()));
		
		const v2<float> ap = a->second.convert<float>();
		const v2<float> bp = b->second.convert<float>();
		//LOG_DEBUG(("%d:%d -> %d:%d", ap.x, ap.y, bp.x, bp.y));
		v2<float> p = ap, d = bp - ap;
		d.normalize();
		p += d * w;
		int len0 = (int)ap.distance(bp); 
		for(int len = len0; len > w; len -= w, p += d * w) {
			const sdlx::Rect &r = (len == len0)? out: (len <= 2 * w ? in:normal );
			surface.copyFrom(*s, r, 
			(int)(p.x - src.x + dst.x + d.x), 
			(int)(p.y - src.y + dst.y + d.y));
		}
	}
}

template<typename T>
static void coord2v(T &pos, const std::string &str) {
	std::string pos_str = str;

	const bool tiled_pos = pos_str[0] == '@';
	if (tiled_pos) { 
		pos_str = pos_str.substr(1);
	}

	TRY {
		pos.fromString(pos_str);
	} CATCH(mrt::formatString("parsing '%s'", str.c_str()).c_str() , throw;)

	if (tiled_pos) {
		v2<int> tile_size = Map->getTileSize();
		pos.x *= tile_size.x;
		pos.y *= tile_size.y;
		//keep z untouched.
	}
}

void IGameMonitor::loadMap(const std::string &campaign, const std::string &name, const bool spawn_objects, const bool skip_loadmap) {
	_campaign = campaign;
	IMap &map = *IMap::get_instance();

	if (!skip_loadmap) {
		map.load(name);
	} else {
		if (!map.loaded())
			throw_ex(("loadMap() called with skip Map::load() flag. Map must be initialized at this point."));
	}

	_waypoints.clear();
	_waypoint_edges.clear();
	
	Config->clearOverrides();
	
	//const v2<int> size = map.getSize();
	for (IMap::PropertyMap::iterator i = map.properties.begin(); i != map.properties.end(); ++i) {
		std::vector<std::string> res;
		mrt::split(res, i->first, ":");
		const std::string &type = res[0];
		
		if (type != "spawn" && type != "object" && type != "waypoint" && 
			type != "edge" && type != "config" && type != "zone" && type != "ambient-sound")
			throw_ex(("unsupported line: '%s'", i->first.c_str()));
		
		if (!spawn_objects && type != "waypoint" && type != "edge" && type != "config")
			continue;
	
		if (type == "ambient-sound") {
			Mixer->startAmbient(i->second);
			continue;
		}
	
		v3<int> pos;
		if (type != "edge" && type != "config") {
			coord2v< v3<int> >(pos, i->second);
		}
	
		/*
		if (pos.x < 0) 
			pos.x += size.x;
		if (pos.y < 0) 
			pos.y += size.y;
		*/
		
		if (type == "spawn") {
			LOG_DEBUG(("spawnpoint: %d,%d,%d", pos.x, pos.y, pos.z));
			v2<int> tile_size = Map->getTileSize();
			pos.x += tile_size.x / 2;
			pos.y += tile_size.y / 2;
			PlayerManager->addSlot(pos);
		} else {
			if (type == "object") {
				LOG_DEBUG(("spawning: object %s, animation %s, pos: %s", res[1].c_str(), res[2].c_str(), i->second.c_str()));
				if (res.size() < 4)
					throw_ex(("'%s' misses an argument", i->first.c_str()));
				res.resize(5);
				Item item(res[1], res[2], i->first, v2<int>(pos.x, pos.y), pos.z);
				item.destroy_for_victory = res[3].substr(0, 19) == "destroy-for-victory";
				if (res[3] == "save-for-victory")
					item.save_for_victory = res[4];
				GameMonitor->add(item);
			} else if (type == "waypoint") {
				if (res.size() < 3)
					throw_ex(("'%s' misses an argument", i->first.c_str()));
				v2<int> tile_size = Map->getTileSize(); //tiled correction
				pos.x += tile_size.x / 2;
				pos.y += tile_size.y / 2;
				LOG_DEBUG(("waypoint class %s, name %s : %d,%d", res[1].c_str(), res[2].c_str(), pos.x, pos.y));
				_waypoints[res[1]][res[2]] = v2<int>(pos.x, pos.y);
				_all_waypoints[res[2]] = v2<int>(pos.x, pos.y);
			} else if (type == "edge") {
				if (res.size() < 3)
					throw_ex(("'%s' misses an argument", i->first.c_str()));
				if (res[1] == res[2])
					throw_ex(("map contains edge from/to the same vertex"));
				_waypoint_edges.insert(WaypointEdgeMap::value_type(res[1], res[2]));
			} else if (type == "config") {
				if (res.size() < 2)
					throw_ex(("'%s' misses an argument", i->first.c_str()));
				
				std::vector<std::string> value;
				mrt::split(value, i->second, ":");
				value.resize(2);
				if (value[0] != "int" && value[0] != "float" && value[0] != "string")
					throw_ex(("cannot set config variable '%s' of type '%s'", res[1].c_str(), value[0].c_str()));
				Var var(value[0]);
				var.fromString(value[1]);

				Config->setOverride(res[1], var);
			} else if (type == "zone") {
				LOG_DEBUG(("%s %s %s", type.c_str(), i->first.c_str(), i->second.c_str()));
				std::vector<std::string> value;
				mrt::split(value, i->second, ":");
				if (value.size() < 2)
					throw_ex(("'%s' misses an argument", i->first.c_str()));
				v3<int> pos;
				v2<int> size;
				coord2v(pos, value[0]);
				coord2v(size, value[1]);
				res.resize(4);
				
				SpecialZone zone(SpecialZone(ZBox(pos, size), res[1], res[2], res[3]));
				zone.area = "hints/" + name;
				PlayerManager->addSpecialZone(zone);
			} 
		}
	}
	
	if (Config->has("map.kill-em-all")) {
		std::string cstr;
		Config->get("map.kill-em-all", cstr, std::string());
		std::vector<std::string> res;
		mrt::split(res, cstr, ",");
		
		std::set<std::string> classes;
		for(size_t i = 0; i < res.size(); ++i) {
			std::string &str = res[i];
			mrt::trim(str);
			if (!str.empty())
				classes.insert(str);
		}
		GameMonitor->killAllClasses(classes);
		LOG_DEBUG(("kill'em all classes: %u", (unsigned)classes.size()));
	}
	
	LOG_DEBUG(("generating matrixes"));
	Map->generateMatrixes();
	
	LOG_DEBUG(("checking waypoint graph..."));
	for(WaypointEdgeMap::const_iterator i = _waypoint_edges.begin(); i != _waypoint_edges.end(); ++i) {
		const std::string &dst = i->second;
		WaypointEdgeMap::const_iterator b = _waypoint_edges.lower_bound(dst);
		if (b == _waypoint_edges.end() || b->first != dst)
			throw_ex(("no edges out of waypoint '%s'", dst.c_str()));
	}
	LOG_DEBUG(("%u items on map, %u waypoints, %u edges", (unsigned)GameMonitor->getItemsCount(), (unsigned)_waypoints.size(), (unsigned)_waypoint_edges.size()));
	Config->invalidateCachedValues();
	
	GET_CONFIG_VALUE("engine.max-time-slice", float, mts, 0.025);
	World->setTimeSlice(mts);
	
	Window->resetTimer();
}

const std::string IGameMonitor::generatePropertyName(const std::string &prefix) {
	//LOG_DEBUG(("prefix: %s", prefix.c_str()));
	IMap::PropertyMap::const_iterator b = Map->properties.lower_bound(prefix);
	int n = 0;
	
	for(IMap::PropertyMap::const_iterator i = b; i != Map->properties.end(); ++i) {
		if (i->first.compare(0, prefix.size(), prefix) != 0) 
			break;
		std::string suffix = i->first.substr(prefix.size());
		if (!suffix.empty() && suffix[0] == ':') {
			int i = atoi(suffix.c_str() + 1);
			if (i > n) 
				n = i;
		}
	}
	
	++n;

	std::string name =  mrt::formatString("%s:%d", prefix.c_str(), n);
	if (Map->properties.find(name) != Map->properties.end()) 
		throw_ex(("failed to generate unique name. prefix: %s, n: %d", prefix.c_str(), n));
	return name;
}

