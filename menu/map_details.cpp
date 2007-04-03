
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
#include "map_details.h"
#include "mrt/exception.h"
#include "config.h"
#include "mrt/fs_node.h"
#include "tooltip.h"
#include "finder.h"
#include "resource_manager.h"
#include "i18n.h"

MapDetails::MapDetails(const int w, const int h) : _map_desc(0) {
	_background.init("menu/background_box.png", w, h);

	_null_screenshot.loadImage(Finder->find("maps/null.png"));
	_small_font = ResourceManager->loadFont("small", true);
}

void MapDetails::getSize(int &w, int &h) const {
	w = _background.w; h = _background.h;
}

bool MapDetails::onMouse(const int button, const bool pressed, const int x, const int y) {
	_tactics.free();
	if (!pressed) 
		return true;
	
	TRY {
		const std::string fname = base + "/" + map + "_tactics.jpg";
		if (mrt::FSNode::exists(fname)) {
			_tactics.loadImage(fname);
			_tactics.convertAlpha();
		}
	} CATCH("loading mini map", {});
	
	return true;
}

void MapDetails::set(const std::string &base, const std::string &map, const std::string &comment) {
	this->base = base;
	this->map = map;
	
	TRY {
		_screenshot.free();
		const std::string fname = base + "/" + map + ".jpg";
		if (mrt::FSNode::exists(fname)) {
			_screenshot.loadImage(fname);
			_screenshot.convertAlpha();
		}
	} CATCH("loading screenshot", {});
	delete _map_desc; 
	_map_desc = NULL;
	
	int mx, my;
	_background.getMargins(mx, my);

	delete _map_desc;	
	_map_desc = new Tooltip(comment, false, _background.w - 2 * mx);
}

void MapDetails::render(sdlx::Surface &surface, const int x, const int y) {
	_background.render(surface, x, y);
	int mx, my;
	_background.getMargins(mx, my);
	
	int yp = my * 3 / 2;

	const sdlx::Surface &screenshot = _screenshot.isNull()?_null_screenshot:_screenshot;
	int xs = (_background.w - screenshot.getWidth()) / 2;
	surface.copyFrom(screenshot, x + xs, y + yp);
	int ys = screenshot.getHeight();
	yp += (ys < 140)?140:ys;
	
	const std::string fname = base + "/" + map + "_tactics.jpg";
	if (mrt::FSNode::exists(fname)) {
		std::string click_here = I18n->get("menu", "view-map");
		int w = _small_font->render(NULL, 0, 0, click_here);
		_small_font->render(surface, x + (_background.w - w) / 2, y + yp, click_here);
	}
	yp += _small_font->getHeight() + 12;

	if (_map_desc)
		_map_desc->render(surface, x + mx, y + yp);
	
	if (!_tactics.isNull()) {
		surface.copyFrom(_tactics, x + _background.w / 2 - _tactics.getWidth() / 2, y + _background.h / 2 - _tactics.getHeight() / 2);
	}
}

MapDetails::~MapDetails() {
	delete _map_desc;
}
