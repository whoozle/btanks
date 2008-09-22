
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
#include "player_picker.h"
#include "container.h"
#include "sdlx/font.h"
#include "resource_manager.h"
#include "chooser.h"
#include "map_desc.h"
#include "menu_config.h"
#include "config.h"
#include "tooltip.h"
#include "label.h"

class SlotLine : public Container {
public : 

	Chooser *_type, *_vehicle;

	MapDesc map;
	int h, ch;
	std::string variant;
	int slot;
	SlotConfig config;
	
	SlotLine(const MapDesc &map, const std::string &variant, const int i, const SlotConfig &config) : 
	_type(NULL), _vehicle(NULL), 
	map(map), variant(variant), slot(i), config(config) {
		
		_font = ResourceManager->loadFont("medium", true);
		h = _font->get_height();
		int w = _font->get_width();

		std::vector<std::string> options;
		options.push_back("?");

		if (variant =="split") {
			options.push_back("PLAYER-1");
			options.push_back("PLAYER-2");			
			options.push_back("AI");
		} else {
			options.push_back("PLAYER");
			options.push_back("AI");
		}

		_type = new Chooser("medium", options);
		
		options.clear();
		options.push_back("?");
		options.push_back("launcher");
		options.push_back("shilka");
		options.push_back("tank");
		options.push_back("machinegunner");
		options.push_back("civilian");
		options.push_back("mortar");
		
		_vehicle = new Chooser("medium", options, "menu/vehicles.png");
		TRY {
			if(!config.type.empty())
				_type->set(config.type);
		} CATCH("SlotLine ctor (set)", {});

		TRY {
			if(!config.vehicle.empty()) 
				_vehicle->set(config.vehicle);
		} CATCH("SlotLine ctor (set)", {});
		
		//LOG_DEBUG(("restriction: %s", map.object_restriction.c_str()));
		if (map.object_restriction.empty()) {
			for(int i = 4; i < _vehicle->size(); ++i)
				_vehicle->disable(i);
		} else {
			TRY {
				_vehicle->set(map.object_restriction);
				int p = _vehicle->get();
				for(int i = 0; i < _vehicle->size(); ++i) {
					if (i != p)
						_vehicle->disable(i);
				}
			} CATCH("set_restriction", {});
		}
		
		int cw;
		_type->get_size(cw, ch);

		add(0, (ch - h) / 3, new Label(_font, mrt::format_string("%d", i + 1)));


		sdlx::Rect p1;
		p1.x = w * 2;
		//p1.y = (_font->get_height() - ch) / 2;
		p1.w = cw;
		p1.h = ch;
		if (ch > h) 
			h = ch;
		
		add(p1.x, p1.y, _type);
		
		sdlx::Rect p2;
		p2.x = p1.x + p1.w + _font->get_width();

		int vcw, vch;
		_vehicle->get_size(vcw, vch);
		if (vch > h) 
			h = vch;
		p2.w = vcw; p2.h = vch;

		add(p2.x, p2.y, _vehicle);
	}

	void tick(const float dt) {
		if (_type->changed()) {
			_type->reset();
			config.type = _type->getValue();
			invalidate();
			//LOG_DEBUG(("type changed"));
		}
		if (_vehicle->changed()) {
			_vehicle->reset();
			config.vehicle = _vehicle->getValue();
			invalidate();
			//LOG_DEBUG(("vehicle changed"));
		}

		if (changed())
			MenuConfig->update(map.name, variant, slot, config);
	}

private: 
	const sdlx::Font *_font;
};

PlayerPicker::PlayerPicker(const int w, const int h) {
	_vehicles = ResourceManager->load_surface("menu/vehicles.png");
}

const std::string PlayerPicker::getVariant() const {
	bool split;
	Config->get("multiplayer.split-screen-mode", split, false);
	return split?"split":std::string();
}

const bool PlayerPicker::validateSlots(const int changed) {
	GET_CONFIG_VALUE("menu.skip-player-validation", bool, spv, false);
	if (spv)
		return false;
	
	const std::string variant = getVariant();
	if (variant == "split") {
		int p1 = 0, p2 = 0;
		bool changed_p1 =  _slots[changed]->config.hasType("player-1");
		bool changed_p2 =  _slots[changed]->config.hasType("player-2");

		for(size_t i = 0; i < _slots.size(); ++i) {
			SlotLine *slot = _slots[i];
			if (slot->config.hasType("player-1"))
				++p1;
			if (slot->config.hasType("player-2"))
				++p2;
		}

		if (p1 == 1 && p2 == 1) 
			return false;
		
		if (p1 > 1) {
			if (changed_p1) 
				changeSlotTypesExcept("player-1", "ai", changed, 0);
			else 
				changeSlotTypesExcept("player-1", "ai", -1, 1);
		}

		if (p2 > 1) {
			if (changed_p2) 
				changeSlotTypesExcept("player-2", "ai", changed, 0);
			else 
				changeSlotTypesExcept("player-2", "ai", -1, 1);
		}

		if (p1 > 1 || p2 > 1)
			return true;

		if (p1 == 0) {
			if (!changeAnySlotTypeExcept("ai", "player-1", changed))
				changeAnySlotTypeExcept("?", "player-1", changed);
		}

		if (p2 == 0) {
			if (!changeAnySlotTypeExcept("ai", "player-2", changed))
				changeAnySlotTypeExcept("?", "player-2", changed);
		}
	} else {
		int p1 = 0;
		bool changed_p1 =  _slots[changed]->config.hasType("player");
		
		for(size_t i = 0; i < _slots.size(); ++i) {
			SlotLine *slot = _slots[i];
			if (slot->config.hasType("player"))
				++p1;
		}
		if (p1 == 0) {
			if (!changeAnySlotTypeExcept("ai", "player", changed))
				changeAnySlotTypeExcept("?", "player", changed);
		} else if (p1 > 1) {
			if (changed_p1) 
				changeSlotTypesExcept("player", "ai", changed, 0);
			else 
				changeSlotTypesExcept("player", "ai", -1, 1);
		}
	}

	return false;
}

const bool PlayerPicker::changeAnySlotTypeExcept(const std::string &what, const std::string &to, const int e) {
	for(int i = 0; i < (int)_slots.size(); ++i) {
		if (i == e) 
			continue;
		
		SlotLine *slot = _slots[i];
		if (slot->config.hasType(what)) {
			slot->_type->set(to);
			return true;
		}
	}
	return false;
}


const bool PlayerPicker::changeSlotTypesExcept(const std::string &what, const std::string &to, const int e, const int skip) {
	int s = skip;
	
	bool found = false;
	for(int i = 0; i < (int)_slots.size(); ++i) {
		if (skip == 0 && i == e) 
			continue;

		if (s && s--) 
			continue;
		
		
		SlotLine *slot = _slots[i];
		if (slot->config.hasType(what)) {
			slot->_type->set(to);
			found = true;
		}
	}
	return found;	
}

void PlayerPicker::tick(const float dt) {
	for(size_t i = 0; i < _slots.size(); ++i) {
		SlotLine *slot = _slots[i];
		if (slot->changed()) {
			slot->reset();
			validateSlots(i);
		}
	}
	Container::tick(dt);
}

void PlayerPicker::set(const MapDesc &map) {
	clear();
	int mx = 16, my = 16;

	std::vector<SlotConfig> config;

	std::string variant = getVariant();
	
	MenuConfig->fill(map.name, variant, config);
	config.resize(map.slots);

	_slots.clear();
	
	int yp = my;
	for(int i = 0; i < map.slots; ++i) {
		SlotLine *line = new SlotLine(map, variant, i, config[i]);
		_slots.push_back(line);
		add(mx, yp, line);
		yp += line->h + 6;
	}
	
}

void PlayerPicker::render(sdlx::Surface &surface, const int x, const int y) const {
	Container::render(surface, x, y);
}

bool PlayerPicker::onKey(const SDL_keysym sym) {
	return Container::onKey(sym);
}

bool PlayerPicker::onMouse(const int button, const bool pressed, const int x, const int y)  {
	return Container::onMouse(button, pressed, x, y);
}
