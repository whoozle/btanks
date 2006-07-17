#ifndef __BT_MENU_H__
#define __BT_MENU_H__
/* Battle Tanks Game
 * Copyright (C) 2006 Battle Tanks team
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

#include "sdlx/ttf.h"
#include "sdlx/rect.h"
#include <sigc++/sigc++.h>
#include <vector>
#include <map>
#include <string>
#include <deque>

namespace sdlx {
class Surface;
}

class MenuItem;

class MainMenu {
public:
	sigc::signal1<void, const std::string &> menu_signal;

	void init(const int w, const int h);
	void setActive(const bool a);
	const bool isActive() const { return _active; }
	void deinit();
	~MainMenu();
	
	void render(sdlx::Surface &dst);
private:
	void onKey(const Uint8 type, const SDL_keysym sym);
	
	void recalculateSizes();

	int _screen_w, _screen_h;
	bool _active;
	sdlx::TTF _font;
	
	typedef std::vector<MenuItem *> ItemList;
	typedef std::map<const std::string, ItemList> MenuMap;
	MenuMap _items;
	
	size_t _active_item;
	std::string _active_menu;
	
	typedef std::pair<size_t, std::string> MenuID;
	std::deque<MenuID> _menu_path;
	sdlx::Rect _menu_size;
};


#endif

