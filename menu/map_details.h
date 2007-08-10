#ifndef BTANKS_MENU_MAP_DETAILS_H__
#define BTANKS_MENU_MAP_DETAILS_H__

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


#include "container.h"
#include "box.h"
#include "sdlx/surface.h"
#include "sdlx/font.h"

class Tooltip;
class MapDesc;

class MapDetails : public Container {
public: 
	MapDetails(const int w, const int h);

	virtual void render(sdlx::Surface &surface, const int x, const int y);
	virtual void getSize(int &w, int &h) const;
	virtual bool onMouse(const int button, const bool pressed, const int x, const int y);
	
	void set(const MapDesc & map_desc);
	~MapDetails();
private: 
	Box _background;
	Tooltip *_map_desc, *_ai_hint;
	std::string base, map;
	
	sdlx::Surface _screenshot, _tactics, _null_screenshot;
	const sdlx::Font *_small_font;
};

#endif

