
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

IMPLEMENT_SINGLETON(GameMonitor, IGameMonitor);

IGameMonitor::IGameMonitor() : _game_over(false), _check_items(0.5, true), _state_timer(false), _timer(0) {}

void IGameMonitor::checkItems(const float dt) {	
	if (_game_over || !_check_items.tick(dt))
		return;
		
	int goal = 0, goal_total = 0;
	
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
			LOG_DEBUG(("respawning item: %s:%s", item.classname.c_str(), item.animation.c_str()));
			Object *o = ResourceManager->createObject(item.classname, item.animation);
			if (item.z) 
				o->setZ(item.z, true);
			o->addOwner(OWNER_MAP);
			
			World->addObject(o, item.position.convert<float>());
			item.id = o->getID();
			item.dead_on = 0;
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
} CATCH("deserialize", throw);
}
