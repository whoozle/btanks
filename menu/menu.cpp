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

#include "menu.h"
#include "window.h"
#include "menuitem.h"
#include "start_server_menu.h"
#include "join_server_menu.h"
#include "options_menu.h"
#include "campaign_menu.h"
#include "i18n.h"

#include "sdlx/surface.h"
#include "sdlx/font.h"
#include "sdlx/color.h"
#include "mrt/logger.h"
#include "mrt/exception.h"
#include "config.h"
#include "resource_manager.h"
#include "menu_config.h"
#include "sound/mixer.h"

#define ITEM_SPACING 10

MainMenu::MainMenu() : _active(false), _active_item(0), _key_active(false) {
	Window->key_signal.connect(sigc::mem_fun(this, &MainMenu::onKey));
	Window->mouse_signal.connect(sigc::mem_fun(this, &MainMenu::onMouse));
	Window->mouse_motion_signal.connect(sigc::mem_fun(this, &MainMenu::onMouseMotion));
	Window->event_signal.connect(sigc::mem_fun(this, &MainMenu::onEvent));
}

void MainMenu::init(const int w, const int h) {
	MenuConfig->load();
	
	_active = true;
	
	LOG_DEBUG(("loading font..."));
	_font = ResourceManager->loadFont("big", false);

	LOG_DEBUG(("loading background..."));
	_background.init("menu/background_box.png", "menu/highlight_big.png", 407, 338);
	_background_area.w = _background.w;
	_background_area.h = _background.h;
	
	LOG_DEBUG(("creating menu..."));
	_active_item = 0;
	_active_menu.clear();
	
	CampaignMenu * cm = new CampaignMenu(this, w, h);
	if (cm->empty()) {
		delete cm;
		cm = NULL;
	} else {
		_items[""].push_back(new MenuItem(_font, "#start-campaign", "submenu", I18n->get("menu", "start-campaign")));
	}

	_items[""].push_back(new MenuItem(_font, "#start-server", "submenu", I18n->get("menu", "start-game")));
	_items[""].push_back(new MenuItem(_font, "#join-server", "submenu", I18n->get("menu", "join-game")));
	_items[""].push_back(new MenuItem(_font, "#options", "submenu", I18n->get("menu", "options")));
	_items[""].push_back(new MenuItem(_font, "credits", "command", I18n->get("menu", "credits")));
	_items[""].push_back(new MenuItem(_font, "quit", "command", I18n->get("menu", "quit")));

	_items[_active_menu][_active_item]->onFocus();
	
	_special_menus["#start-server"] = new StartServerMenu(this, w, h);
	_special_menus["#join-server"] = new JoinServerMenu(this, w, h);
	_special_menus["#options"] = new OptionsMenu(this, w, h);

	if (cm != NULL) {
		_special_menus["#start-campaign"] = cm;
		cm = NULL;
	}

	recalculateSizes();

}

void MainMenu::recalculateSizes() {
	_menu_size.x = _menu_size.y = 0;
	for(ItemList::const_iterator i = _items[_active_menu].begin(); i != _items[_active_menu].end(); ++i) {
		int w, h;
		(*i)->getSize(w, h);
		if (w > _menu_size.x) 
			_menu_size.x = w;
		_menu_size.y += h + ITEM_SPACING;
	}
}

void MainMenu::tick(const float dt) {
	if (!_active)
		return;
		
	{
		static float timer; 
		if (_key_active) {
			timer += dt;
			if (timer >= 0.25) {
				onKey(_key_emulated, true);
				onKey(_key_emulated, false);
				timer = 0;
			}
		} else 
			timer = 0;
	}
	
/*	for(std::map<const std::string, BaseMenu *>::iterator i = _special_menus.begin(); i != _special_menus.end(); ++i) {
		if (i->second)
			i->second->tick(dt);
	}
*/
	std::map<const std::string, BaseMenu *>::iterator i = _special_menus.find(_active_menu);
	if (i != _special_menus.end() && i->second != NULL)
		i->second->tick(dt);
}


void MainMenu::deinit() {
	for(MenuMap::iterator m = _items.begin(); m != _items.end(); ++m) {
		for(ItemList::iterator i = m->second.begin(); i != m->second.end(); ++i) {
			delete *i;
			*i = NULL;
		}
	}
	_items.clear();
	for(std::map<const std::string, BaseMenu *>::iterator i = _special_menus.begin(); i != _special_menus.end(); ++i) {
		delete i->second;
	}
	_special_menus.clear();
	_menu_path.clear();
	_active_menu.clear();
	_active_item = 0;
	
	MenuConfig->save();
}

MainMenu::~MainMenu() { 
	LOG_DEBUG(("cleaning up menus..."));
	deinit(); 
}

void MainMenu::activateSelectedItem() {
	MenuItem * item = _items[_active_menu][_active_item];
	assert(item != NULL);
		
	const std::string &name = item->name;
	if (item->type == "submenu") {
		Mixer->playSample(NULL, "menu/select.ogg", false);
		LOG_DEBUG(("entering submenu '%s'", name.c_str()));
		if (name[0] == '#') {
			_menu_path.push_front(MenuID(_active_item, _active_menu));
			_active_menu = name;
			return;
		}
		if (_items[name].empty())
			throw_ex(("no submenu %s found or it's empty", name.c_str()));

		_menu_path.push_front(MenuID(_active_item, _active_menu));
		_items[_active_menu][_active_item]->onLeave();
		_active_menu = name;
		_active_item = 0;
		_items[_active_menu][_active_item]->onFocus();
		recalculateSizes();
	} else if (item->type == "back") {
		if (!back()) 
			throw_ex(("cannot do 'back' command from top-level menu"));
	} else if (item->type == "command") {
		LOG_DEBUG(("command: %s", name.c_str()));
		menu_signal.emit(name, item->getValue());
	} else if (item->type == "iterable") {
		item->onClick();
		recalculateSizes();
	} else if (item->type == "text") {
		item->onClick();
	} else throw_ex(("unknown menu item type: %s", item->type.c_str()));
}


void MainMenu::up() {
	Mixer->playSample(NULL, "menu/move.ogg", false);

			_items[_active_menu][_active_item]->onLeave();

			if (_active_item == 0) 
				_active_item = _items[_active_menu].size() - 1;
			else --_active_item;
			_items[_active_menu][_active_item]->onFocus();

}

void MainMenu::down() {
	Mixer->playSample(NULL, "menu/move.ogg", false);
	
			_items[_active_menu][_active_item]->onLeave();
			if (_active_item == _items[_active_menu].size() - 1) 
				_active_item = 0;
			else ++_active_item;
			_items[_active_menu][_active_item]->onFocus();
}

bool MainMenu::onKey(const SDL_keysym sym, const bool pressed) {
	if (!_active || !pressed)
		return false;
		
	BaseMenu * bm = getMenu(_active_menu);
	if (bm != NULL) {
		return bm->onKey(sym);
	}
	
	if (_items[_active_menu].empty())
		throw_ex(("no menu '%s' found", _active_menu.c_str()));
	MenuItem * item = _items[_active_menu][_active_item];
	if (item->onKey(sym))
		return true;
	
	switch(sym.sym) {
		case SDLK_UP:
			up();
			return true;

		case SDLK_DOWN:
			down();
			return true;

		case SDLK_RETURN: 
			activateSelectedItem();
			return true;
		case SDLK_ESCAPE: 
			return false;
		default: 
			break;
	}
	//LOG_DEBUG(("active item = %u", _active_item));
	return false;
}

#include "resource_manager.h"

void MainMenu::render(sdlx::Surface &dst) {
	if (!_active)
		return;
		
	BaseMenu * sm = getMenu(_active_menu);
	if (sm != NULL) {
		sm->render(dst, 0, 0);
		return;
	}

	int base_x = (dst.getWidth() - _background.w) / 2, base_y = (dst.getHeight() - _background.h) / 2;
	_background.render(dst, base_x, base_y);
	
	int x = (dst.getWidth() - _menu_size.x) /2;
	int y = (dst.getHeight() - _menu_size.y) / 2;

	_background_area.x = x;
	_background_area.y = y;
	
	const ItemList & items = _items[_active_menu];
	size_t n = items.size();
	for(size_t i = 0; i < n ;++i) {
		int w,h;
		items[i]->getSize(w, h);

		if (_active_item == i) {
			_background.renderHL(dst, base_x, y + h / 2);
		}
	
		items[i]->render(dst, x + (_menu_size.x - w) / 2, y);
		y += h + ITEM_SPACING;
	}
}

#include "sdlx/cursor.h"

void MainMenu::setActive(const bool a) {
	_active = a;
	if (a) {
		sdlx::Cursor::Enable();
	} else {
		sdlx::Cursor::Disable();
	}
}

void MainMenu::reset() {
	_items[_active_menu][_active_item]->onLeave();
	_menu_path.clear();
	_active_menu.clear();
	_active_item = 0;
	_items[_active_menu][_active_item]->onFocus();
	recalculateSizes();
}

const bool MainMenu::back() {
	if (_menu_path.size() == 0) 
		return false;
	
	Mixer->playSample(NULL, "menu/return.ogg", false);
	
	if (_active_menu[0] != '#')
		_items[_active_menu][_active_item]->onLeave();
	
	_active_item = _menu_path.front().first;
	_active_menu = _menu_path.front().second;
	
	_menu_path.pop_front();
	
	if (!_active_menu.empty() && _active_menu[0] != '#')
		_items[_active_menu][_active_item]->onFocus();
	
	recalculateSizes();
	return true;
}

bool MainMenu::onMouseMotion(const int state, const int x, const int y, const int xrel, const int yrel) {
	if (!_active)
		return false;
	
	BaseMenu * bm = getMenu(_active_menu);
	if (bm != NULL) {
		return bm->onMouseMotion(state, x, y, xrel, yrel);
	}
	return false;
}

bool MainMenu::onMouse(const int button, const bool pressed, const int x, const int y) {
	if (!_active)
		return false;
	
	BaseMenu * bm = getMenu(_active_menu);
	if (bm != NULL) {
		return bm->onMouse(button, pressed, x, y);
	}
	if (!pressed)
		return false;

	if (button == SDL_BUTTON_WHEELUP) {
		up();
		return true;
	} else if (button == SDL_BUTTON_WHEELDOWN) {
		down();		
		return true;
	}

	if (_background_area.in(x, y)) {
		sdlx::Rect item_area = _background_area;
		const ItemList & items = _items[_active_menu];
		for(size_t i = 0; i < items.size() ;++i) {
			int w, h;
			items[i]->getSize(w, h);
			item_area.h = h;
			
			if (item_area.in(x, y)) {
				_active_item = i;
				LOG_DEBUG(("clicked item %u", (unsigned)i));
				activateSelectedItem();
				return true;
			}

			item_area.y += h + ITEM_SPACING;
		}
		return false;
	}
	//LOG_DEBUG(("%d %c %d %d", button, pressed?'+':'-', x, y));
	return false;
}

BaseMenu *MainMenu::getMenu(const std::string &menu) {
	return _special_menus[menu];
}

#include "math/unary.h"

void MainMenu::onEvent(const SDL_Event &e) {
	if (!_active)
		return;

	SDL_keysym sym;
	memset(&sym, 0, sizeof(sym));
	sym.mod = KMOD_NONE;
	
	if (e.type == SDL_JOYBUTTONDOWN || e.type == SDL_JOYBUTTONUP) {
		sym.sym = (e.jbutton.button == 0)?SDLK_RETURN:SDLK_ESCAPE;
		onKey(sym, e.type == SDL_JOYBUTTONDOWN);
	} else if (e.type == SDL_JOYHATMOTION) {
		if (e.jhat.value & SDL_HAT_UP) {
			sym.sym = SDLK_UP;
			onKey(sym, true);
		} else if (e.jhat.value & SDL_HAT_DOWN) {
			sym.sym = SDLK_DOWN;
			onKey(sym, true);
		} else if (e.jhat.value & SDL_HAT_LEFT) {
			sym.sym = SDLK_LEFT;
			onKey(sym, true);
		} else if (e.jhat.value & SDL_HAT_RIGHT) {
			sym.sym = SDLK_RIGHT;
			onKey(sym, true);
		}
	} else if (e.type == SDL_JOYAXISMOTION && e.jaxis.axis < 4) {
#define M (32768 - 3276)
		static int value[4] = {0,0,0,0};
		const int a = e.jaxis.axis;
		const int v = e.jaxis.value;
		//LOG_DEBUG(("%d: %d %d", a, value[a], v));
		if (math::abs(value[a]) <= M && math::abs(v) > M) {
			sym.sym = v > 0 ? SDLK_DOWN: SDLK_UP;
			onKey(sym, true);
			value[a] = v;
			_key_active = true;
			_key_emulated = sym;
		} else if (math::abs(value[a]) > M && math::abs(v) <= M) {
			sym.sym = value[a] > 0 ? SDLK_DOWN: SDLK_UP;
			onKey(sym, false);
			value[a] = v;
			_key_active = false;
		}
	}
}
