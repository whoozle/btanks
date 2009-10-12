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

#include "open_map_dialog.h"
#include "finder.h"
#include "resource_manager.h"
#include "i18n.h"
#include "mrt/directory.h"
#include "mrt/file.h"

#include "menu/box.h"
#include "menu/button.h"
#include "menu/chooser.h"
#include "menu/prompt.h"
#include "menu/text_control.h"
#include "menu/number_control.h"

#include "tmx/map.h"

void OpenMapDialog::load() {
	//LOG_DEBUG(("map: %s %s", c_base->getValue().c_str(), c_map->getValue().c_str()));
	base = c_base->getValue();
	map = c_map->getValue();

	invalidate();
	hide();
}

void OpenMapDialog::getMap(std::string &dir, std::string &name) const {
	dir = base; name = map;
}

void OpenMapDialog::tick(const float dt) {
	Container::tick(dt);
	if (b_back->changed()) {
		b_back->reset();

		if (!Map->loaded())
			return;

		hide();
	}
	if (b_ok->changed()) {
		b_ok->reset();
		load();
	}
	if (c_base->changed()) {
		c_base->reset();
		
		c_map->hide();
		c_map = _map_chooser[c_base->getValue()];
		c_map->hide(false);
	}
	if (b_new->changed()) {
		b_new->reset();
		p_name->set(std::string());
		p_name->hide(false);
	}
	if (p_name->changed()) {
		map = p_name->get();
		LOG_DEBUG(("name invalidated: '%s'", map.c_str()));

		p_name->reset();
		p_name->hide();
	
		base = c_base->getValue();
		if (!map.empty()) {
			LOG_DEBUG(("creating new map with name '%s/%s'", base.c_str(), map.c_str()));
			TRY {
				mrt::File file;
				try {
					mrt::Directory dir;
					dir.create(base + "/maps", true);
				} catch(...) {}

				file.open(mrt::format_string("%s/maps/%s.tmx", base.c_str(), map.c_str()), "wb");
				file.write_all(mrt::format_string(
					"<?xml version=\"1.0\"?>\n"
					"<map version=\"0.99b\" orientation=\"orthogonal\" width=\"%d\" height=\"%d\" tilewidth=\"64\" tileheight=\"64\">\n"
					"</map>\n", n_width->get(), n_height->get()
					)
				);
				file.close();
			} CATCH("writing new map", return);
			invalidate();
			hide();
		}
	}
}

bool OpenMapDialog::onKey(const SDL_keysym sym) {
	switch(sym.sym) {
	case SDLK_PAGEDOWN:
		c_base->left();
		return true;

	case SDLK_PAGEUP:
		c_base->right();
		return true;
	
	case SDLK_LEFT: 
	case SDLK_UP: 
		c_map->left();
		return true;

	case SDLK_RIGHT: 
	case SDLK_DOWN: 
		c_map->right();
		return true;

	case SDLK_KP_ENTER:
	case SDLK_RETURN:
		if (Container::onKey(sym))
			return true;
		
		hide();
		load();
		return true;
	
	default: 
		return Container::onKey(sym);
	}
}

OpenMapDialog::OpenMapDialog() {
	std::vector<std::string> path;
	Finder->getPath(path);

	for(size_t i = 0; i < path.size(); ++i) {
	TRY {
		std::vector<std::string> entries;
		LOG_DEBUG(("scanning directory %s", path[i].c_str()));
		Finder->enumerate(entries, path[i], "maps");

		for(size_t j = 0; j < entries.size(); ++j) {
			std::string map = entries[j];
		
			mrt::to_lower(map);
			if (map.size() < 5 || map.compare(map.size() - 4, 4, ".tmx") != 0)
				continue;
			map = map.substr(0, map.size() - 4);
			LOG_DEBUG(("found map: %s", map.c_str()));
			_maps.insert(std::pair<const std::string, std::string>(path[i], map));
		}	
	} CATCH("scan", {})
	}

	//creating GUI
	int w = 320, h = 200;
	Box *b = new Box("menu/background_box.png", w, h);
	add(0, 0, b);
	get_size(w, h);
	
	int mx, my;
	b->getMargins(mx, my);
	w -= mx;
	h -= my;

	int cw, ch;
	c_base = new Chooser("small", path);
	c_base->get_size(cw, ch);

	int yp = my;
	add(mx + (w - cw) / 2, yp, c_base);
	yp += ch;

	TRY {
		c_base->set(Finder->get_base_path());
	} CATCH("ctor", {})
	
	for(size_t i = 0; i < path.size(); ++i) {
		std::multimap<const std::string, std::string>::const_iterator b = _maps.lower_bound(path[i]);
		std::multimap<const std::string, std::string>::const_iterator e = _maps.upper_bound(path[i]);
		std::vector<std::string> submaps;
		for(std::multimap<const std::string, std::string>::const_iterator j = b; j != e; ++j) {
			submaps.push_back(j->second);
		}
		Chooser *c;
		_map_chooser[path[i]] = c = new Chooser("small", submaps);
		c->hide();
	}


	for(std::map<const std::string, Chooser *>::const_iterator i = _map_chooser.begin(); i != _map_chooser.end(); ++i) {
		int cw, ch;
		i->second->get_size(cw, ch);
		add(mx + (w - cw) / 2, yp, i->second);
	}

	c_map = _map_chooser[c_base->getValue()];
	c_map->hide(false);
	c_map->get_size(cw, ch);
	yp += ch;
	
	n_width = new NumberControl("small", 20);
	n_height = new NumberControl("small", 20);

	n_width->get_size(cw, ch);
	
	add(w / 2 - cw - mx, yp, n_width);
	add(w / 2 + cw + mx, yp, n_height);

	int b1, b2, b3;

	b_ok = new Button("small", I18n->get("menu", "ok"));
	b_ok->get_size(b1, ch);

	b_new = new Button("small", I18n->get("editor", "new-map"));
	b_new->get_size(b2, ch);

	b_back = new Button("small", I18n->get("menu", "back"));
	b_back->get_size(b3, ch);
	
	
	int w1 = w * b1 / (b1 + b2 + b3);
	int w2 = w * b2 / (b1 + b2 + b3);
	int w3 = w * b3 / (b1 + b2 + b3);
	int offset = mx / 2;

	add(offset + w1 / 2 - b1 / 2, h - my - ch, b_ok);
	add(offset + w1 + w2 / 2 - b2 / 2, h - my - ch, b_new);
	add(offset + w1 + w2 + w3 / 2 - b3 / 2, h - my - ch, b_back);
	
	p_name = new Prompt(w, h * 2 / 3, new TextControl("small"));
	p_name->get_size(cw, ch);
	p_name->hide();
	add((w - cw) / 2, (h - ch) / 2, p_name);
}

