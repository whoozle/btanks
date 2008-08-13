#ifndef BTANKS_EDITOR_TILESET_DIALOG_H__
#define BTANKS_EDITOR_TILESET_DIALOG_H__

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

#include "menu/container.h"
#include <vector>
#include <string>
#include <map>

#include "sdlx/surface.h"
#include "tmx/tileset_list.h"
#include "base_brush.h"
#include "sl08/sl08.h"

class Box;
class ScrollList;
class AddTilesetDialog;

class TilesetDialog : public Container {
public:
	TilesetDialog(const int w, const int h);
	virtual bool onKey(const SDL_keysym sym);
	virtual void render(sdlx::Surface &surface, const int x, const int y) const;
	virtual void get_size(int &w, int &h) const { w = _w; h = _h; }
	
	BaseBrush *getBrush();
	const bool tileset_added() { bool r = _tileset_added; _tileset_added = false; return r; }

private: 
	void set(const int tileset);
	void initMap();
	sl08::slot0<void, TilesetDialog> init_map_slot;
	
	virtual void tick(const float dt);
	virtual bool onMouse(const int button, const bool pressed, const int x, const int y);
	virtual bool onMouseMotion(const int state, const int x, const int y, const int xrel, const int yrel);

	int _w, _h;
	
	std::vector<std::string> _all_tilesets;
	TilesetList _tilesets;
	const sdlx::Surface *_current_tileset;
	int _current_tileset_idx, _current_tileset_gid;
	
	Box *_tileset_bg;
	ScrollList *_sl_tilesets;
	v2<float> _vel, _pos;

	//brush stuff
	bool _selecting, _selected;
	v2<int> _brush_1, _brush_2;
	sdlx::Surface _brush;
	Brush _editor_brush;

	//map stuff
	std::string _fname;
	v2<int> _tile_size;
	
	AddTilesetDialog *_add_tileset;
	bool _tileset_added;
};

#endif

