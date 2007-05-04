#ifndef __BT_MENU_H__
#define __BT_MENU_H__

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

#include "sdlx/rect.h"
#include <sigc++/sigc++.h>
#include <vector>
#include <map>
#include <string>
#include <deque>
#include "box.h"
#include "math/v2.h"

namespace sdlx {
	class Font;
}

class MenuItem;
class BaseMenu;

class MainMenu : public sigc::trackable {
public:
	MainMenu(const int w, const int h);

	sigc::signal2<void, const std::string &, const std::string &> menu_signal;

	void setActive(const bool a);
	const bool isActive() const { return _active; }
	void deinit();
	~MainMenu();
	
	void tick(const float dt);
	void render(sdlx::Surface &dst);
	void reset();
	const bool back();
	
private:
	std::map<const std::string, BaseMenu *> _special_menus;
	BaseMenu *getMenu(const std::string &menu);

	bool onKey(const SDL_keysym sym, const bool pressed);
	bool onMouse(const int button, const bool pressed, const int x, const int y);
	bool onMouseMotion(const int state, const int x, const int y, const int xrel, const int yrel);
	
	void recalculateSizes();
	void activateSelectedItem();

	bool _active;
	const sdlx::Font *_font;
	
	typedef std::vector<MenuItem *> ItemList;
	typedef std::map<const std::string, ItemList> MenuMap;
	MenuMap _items;
	
	size_t _active_item;
	std::string _active_menu;
	
	typedef std::pair<size_t, std::string> MenuID;
	std::deque<MenuID> _menu_path;
	v2<int> _menu_size;	
	
	Box _background;
	sdlx::Rect _background_area;
};


#endif

