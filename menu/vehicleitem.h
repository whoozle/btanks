#ifndef __BTANKS_MENU_VEHICLE_ITEM_H__
#define __BTANKS_MENU_VEHICLE_ITEM_H__

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

#include "menuitem.h"
#include <vector>

class VehicleItem : public MenuItem {
public: 
	VehicleItem(const sdlx::Font *font, const std::string &name, const std::string &subkey);
	virtual void onClick();
	virtual const bool onKey(const SDL_keysym sym);
	void render(sdlx::Surface &dst, const int x, const int y);
	
private:
	void updateValue();

	void finish();
	bool _active;
	size_t _index;
	
	std::vector<std::string> _items;
	std::string _subkey;
};


#endif

