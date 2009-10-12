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

#include <algorithm>
#include "tileset_dialog.h"
#include "finder.h"
#include "mrt/directory.h"
#include "menu/box.h"
#include "resource_manager.h"
#include "sdlx/surface.h"
#include "tmx/map.h"
#include "math/binary.h"
#include "config.h"
#include "menu/scroll_list.h"
#include "base_brush.h"
#include "add_tileset_dialog.h"
#include "mrt/fs_node.h"

TilesetDialog::TilesetDialog(const int w, const int h) : 
	_w(w), _h(h), _current_tileset(NULL), _current_tileset_idx(-1), _current_tileset_gid(0), _selecting(false), _selected(false), 
	_tileset_added(false)
	{
	init_map_slot.assign(this, &TilesetDialog::initMap, Map->load_map_signal);

	std::vector<std::string> path;
	Finder->getPath(path);
	std::reverse(path.begin(), path.end());
	
	for(size_t i = 0; i < path.size(); ++i) {
		LOG_DEBUG(("%u: %s", (unsigned)i, path[i].c_str()));
		TRY {
			std::vector<std::string> dir;
			Finder->enumerate(dir, path[i], "/tilesets");
			for(size_t j = 0; j < dir.size(); ++j) {
				std::string fname = dir[j];
				if (fname.size() < 5 || fname.compare(fname.size() - 4, 4, ".png") != 0) 
					continue;
				fname.insert(0, path[i] + "/tilesets/");
				_all_tilesets.push_back(fname);
				LOG_DEBUG(("found tileset %s", fname.c_str()));
			}
		} CATCH("scanning tilesets directory", continue);
	}
	add(0, 0, _tileset_bg = new Box("transparent_background_box.png", w, h));

	_sl_tilesets = new ScrollList("menu/background_box.png", "small", 200, h);
	int cw, ch;
	_sl_tilesets->get_size(cw, ch);
	add(w - cw, 0, _sl_tilesets);

	_add_tileset = new AddTilesetDialog(200, h);
	_add_tileset->get_size(cw, ch);
	add(w - cw, 0, _add_tileset);
	_add_tileset->hide();
}

void TilesetDialog::initMap() {
	_fname = Finder->find("maps/" + Map->getName() + ".tmx");
	LOG_DEBUG(("fixme: finder returns %s", _fname.c_str()));
	
	_tile_size = Map->getTileSize();
	_brush.create_rgb(_tile_size.x, _tile_size.y, 32);
	_brush.display_format_alpha();
	_brush.fill_rect(_brush.get_size(), _brush.map_rgba(0xdd, 0xdd, 0x11, 0x80));
	_tilesets = Map->getTilesets();
	
	_sl_tilesets->clear();
	size_t n = _tilesets.size();
	for(size_t i = 0; i < n; ++i) {
		std::string name = _tilesets[i].first;
		{
			//beautify name
			name = mrt::FSNode::get_filename(name, false);
		}
		
		char key = '1' + (char)i;
		if (i == 9) 
			key = '0'; 
		if (i >= 10)
			key = 'a' + i - 10;
		_sl_tilesets->append(mrt::format_string("%c.%s", key, name.c_str()));
	}
}

bool TilesetDialog::onMouse(const int button, const bool pressed, const int x, const int y) {
	if (!Map->loaded())
		return false;
	if (_current_tileset != NULL && x <= _current_tileset->get_width()) {
	switch(button) {
	case SDL_BUTTON_LEFT: 
		if (pressed) {
			_brush_2 = _brush_1 = (_pos.convert<int>() + v2<int>(x, y)) / _tile_size;
			//add validation
			_selecting = true;
		} else if (_selecting /*&&!pressed*/){
			_selecting = false;
			_selected = true;
			
			int y0 = math::min(_brush_1.y, _brush_2.y);
			int y1 = math::max(_brush_1.y, _brush_2.y);
			int x0 = math::min(_brush_1.x, _brush_2.x);
			int x1 = math::max(_brush_1.x, _brush_2.x);
			int tileset_width = (_current_tileset->get_width() - 1) / _tile_size.x + 1;
			
			LOG_DEBUG(("brush : %dx%d-%dx%d", x0, y0, x1, y1));
			std::vector<int> tiles;

			for(int yb = y0; yb <= y1; ++yb) {
				for(int xb = x0; xb <= x1; ++xb) {
					int tid = _current_tileset_gid + xb + yb * tileset_width;
					//LOG_DEBUG(("adding tile %d to brush (base %d)", tid, _current_tileset_gid));
					tiles.push_back(tid);
				}
			}

			_editor_brush = Brush(_tile_size, tiles);
			_editor_brush.size = v2<int>(x1 - x0 + 1, y1 - y0 + 1);
			invalidate();
			
			static const Uint8 *keys = SDL_GetKeyState(0);
			if (keys[SDLK_LCTRL] == 0)
				hide();
		}
	break;
	
	case SDL_BUTTON_RIGHT: 
		_selected = false;
	break;
	
	case SDL_BUTTON_WHEELUP: 
		if (_current_tileset) 
			_pos.y -= _tile_size.y;
	break;
	
	case SDL_BUTTON_WHEELDOWN: 
		if (_current_tileset) 
			_pos.y += _tile_size.y;
	break;
	}
	
	return true;
	}
	
	return Container::onMouse(button, pressed, x, y);
}

bool TilesetDialog::onMouseMotion(const int state, const int x, const int y, const int xrel, const int yrel) {
	if (!Map->loaded())
		return false;
	if (_current_tileset && _current_tileset->get_height() > _h) {
		if (y + _tile_size.y / 2 >= _h) {
			_vel.y = 1;
		} else if (y <= _tile_size.y / 2) {
			_vel.y = -1;
		} else _vel.y = 0;
	}
	
	if (_selecting) {
		_brush_2 = (_pos.convert<int>() + v2<int>(x, y)) / _tile_size;
	}
	return true;
}

void TilesetDialog::tick(const float dt) {
	Container::tick(dt);
	if (_sl_tilesets->changed()) {
		_sl_tilesets->reset();
		if (!_sl_tilesets->empty())
			set(_sl_tilesets->get());
	}

	std::string tileset = _add_tileset->getTileset();
	if (!tileset.empty()) {
		LOG_DEBUG(("adding tileset!"));
		Map->addTileset(tileset);
		initMap();
		_tileset_added = true;
	}

	if (_current_tileset == NULL || hidden())
		return;

	GET_CONFIG_VALUE("editor.scrolling-speed", int, ss, 500);
	_pos += _vel * (ss * dt);
	if (_pos.x + _w > _current_tileset->get_width())
		_pos.x = _current_tileset->get_width() - _w;
	if (_pos.y + _h > _current_tileset->get_height())
		_pos.y = _current_tileset->get_height() - _h;

	if (_pos.x < 0)
		_pos.x  = 0;
	if (_pos.y < 0)
		_pos.y  = 0;
	//LOG_DEBUG(("%g %g", _pos.x, _pos.y));
	
}


bool TilesetDialog::onKey(const SDL_keysym sym) {
	if (Container::onKey(sym)) {
		return true;
	}

	//LOG_DEBUG(("sym.sym == '%c' (%02x), mod: %04x", (char)sym.sym, (unsigned)sym.sym, (unsigned)sym.mod));
	if ((sym.sym == SDLK_ESCAPE || sym.sym == SDLK_TAB) && !hidden()) {
		hide();
		return true;
	}
	
	if (sym.sym == SDLK_n) {
		if (_add_tileset->init(_fname, _tilesets, _all_tilesets))
			_add_tileset->hide(false);
		return true;
	}
	
	if (sym.mod & (KMOD_CTRL | KMOD_ALT | KMOD_META | KMOD_SHIFT))
		return false;

	int cd = (int)(sym.sym - SDLK_0);
	int cc = (int)(sym.sym - SDLK_a);
	int tileset = -1;
	if(cd >= 0 && cd <= 9) 
		tileset = cd?cd-1:9;
	else if (cc >= 0 && cc < 26) {
		tileset = cc + 10;
	} 
	
	//LOG_DEBUG(("pop tileset %d", tileset));
	if (hidden()) {
		hide(false);
	} else if (tileset == _current_tileset_idx) {
		hide();
		return true;
	}
	set(tileset);
	return true;
}

void TilesetDialog::set(const int tileset) {
	if (tileset < 0 || tileset >= (int)_tilesets.size())
		return;
	if (tileset != _current_tileset_idx) {
		//do it per tileset.
		_brush_1.clear();
		_brush_2.clear();
		_pos.clear();
		_vel.clear();
	}
	
	std::string name = _tilesets[tileset].first;
	_current_tileset_idx = tileset;
	_current_tileset_gid = _tilesets[tileset].second;

	_current_tileset = ResourceManager->load_surface("../tilesets/" + name);
	_tileset_bg->init("transparent_background_box.png", _current_tileset->get_width(), _current_tileset->get_height());
}

void TilesetDialog::render(sdlx::Surface &surface, const int x, const int y) const {
	Container::render(surface, x, y);
	if (_current_tileset == NULL)
		return;
	surface.blit(*_current_tileset, -(int)_pos.x, -(int)_pos.y);

	if (!_brush.isNull() && (_selected || _selecting)) {
		int y0 = math::min(_brush_1.y, _brush_2.y);
		int y1 = math::max(_brush_1.y, _brush_2.y);
		int x0 = math::min(_brush_1.x, _brush_2.x);
		int x1 = math::max(_brush_1.x, _brush_2.x);

		for(int yb = y0; yb <= y1; ++yb) {
			for(int xb = x0; xb <= x1; ++xb) {
				surface.blit(_brush, x + xb * _tile_size.x - (int)_pos.x, y + yb * _tile_size.y - (int)_pos.y);
			}
		}
	}
}

BaseBrush *TilesetDialog::getBrush() {
	return new Brush(_editor_brush);
}
