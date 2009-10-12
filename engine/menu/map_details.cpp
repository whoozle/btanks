
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
#include "map_details.h"
#include "mrt/exception.h"
#include "config.h"
#include "tooltip.h"
#include "finder.h"
#include "resource_manager.h"
#include "i18n.h"
#include "map_desc.h"
#include "mrt/chunk.h"

MapDetails::MapDetails(const int w, const int h) : _w(w), _h(h), _map_desc(0), _ai_hint(NULL), has_tactics(false) {
	mrt::Chunk data;
	Finder->load(data, "maps/null.png");
	_null_screenshot.load_image(data);
	_null_screenshot.display_format_alpha();
	_small_font = ResourceManager->loadFont("small", true);

/*
	if (hint && I18n->has("tips", "deathmatch-bots")) {
		int mw, mh;
		get_size(mw, mh);
		_ai_hint = new Tooltip("tips", "deathmatch-bots", true, w);
		int tw, th;
		_ai_hint->get_size(tw, th);
		add((mw - tw) / 2, mh + 2, _ai_hint);
	}
*/
}

void MapDetails::get_size(int &w, int &h) const {
	w = _w; h = _h;
}

bool MapDetails::onMouse(const int button, const bool pressed, const int x, const int y) {
	_tactics.free();
	if (!pressed) 
		return true;
	
	TRY {
		std::string fname = "maps/" + map + "_tactics.jpg";
		if (Finder->exists(base, fname)) {
			mrt::Chunk data;
			Finder->load(data, fname);
			_tactics.load_image(data);
			_tactics.display_format_alpha();
			has_tactics = true;
		}
	} CATCH("loading tactic map", {});
	
	return true;
}

void MapDetails::set(const MapDesc & map_desc) {
	base = map_desc.base;
	map = map_desc.name;

	//LOG_DEBUG(("selected base: %s, map: %s", base.c_str(), map.c_str()));
	
	TRY {
		_screenshot.free();
		const std::string fname = "maps/" + map + ".jpg";
		if (Finder->exists(base, fname)) {
			mrt::Chunk data;
			Finder->load(data, fname);
			_screenshot.load_image(data);
			_screenshot.display_format_alpha();
		}
	} CATCH("loading screenshot", {});

	std::string fname = "maps/" + map + "_tactics.jpg";
	has_tactics = Finder->exists(base, fname);

	delete _map_desc; 
	_map_desc = NULL;
	
	delete _map_desc;
	//const std::string &comments = I18n->has("maps/descriptions", map)?I18n->get("maps/descriptions", map): 
	//I18n->get("maps/descriptions", "(default)");
	
	_map_desc = new Tooltip("maps/descriptions", I18n->has("maps/descriptions", map)? map:"(default)" , false, _w);
	if (_ai_hint != NULL) {
		_ai_hint->hide(map_desc.game_type != GameTypeDeathMatch);
	}
}

void MapDetails::render(sdlx::Surface &surface, const int x, const int y) const {
	Container::render(surface, x, y);
	
	int mx = 16, my = 16;
	int yp = my * 3 / 2;

	const sdlx::Surface &screenshot = _screenshot.isNull()?_null_screenshot:_screenshot;
	int xs = (_w - screenshot.get_width()) / 2;
	surface.blit(screenshot, x + xs, y + yp);
	int ys = screenshot.get_height();
	yp += (ys < 140)?140:ys;
	
	if (has_tactics) {
		std::string click_here = I18n->get("menu", "view-map");
		int w = _small_font->render(NULL, 0, 0, click_here);
		_small_font->render(surface, x + (_w - w) / 2, y + yp, click_here);
	}
	yp += _small_font->get_height() + 12;

	if (_map_desc)
		_map_desc->render(surface, x + mx, y + yp);
	
	if (!_tactics.isNull()) {
		surface.blit(_tactics, x + _w / 2 - _tactics.get_width() / 2, y + _h / 2 - _tactics.get_height() / 2);
	}
}

MapDetails::~MapDetails() {
	delete _map_desc;
}
