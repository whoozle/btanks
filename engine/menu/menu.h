#ifndef __BT_MENU_MAINMENU_H__
#define __BT_MENU_MAINMENU_H__

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

#include "container.h"
#include "box.h"

class MenuItem;

class Menu : public Container {
public:
	Menu();
	void add(MenuItem *item);
	virtual void render(sdlx::Surface &surface, const int x, const int y) const;
	
	bool onKey(const SDL_keysym sym);
	bool onMouse(const int button, const bool pressed, const int x, const int y);
	bool onMouseMotion(const int state, const int x, const int y, const int xrel, const int yrel);
	
	void up();
	void down();

protected: 
	Box background;
	int spacing;
	
	const Control * get_current_item() const;
	Control * get_current_item();

	int current_item;
private: 
	int width, height;
};

#endif

