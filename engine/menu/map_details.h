#ifndef BTANKS_MENU_MAP_DETAILS_H__
#define BTANKS_MENU_MAP_DETAILS_H__

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


#include "container.h"
#include "sdlx/surface.h"
#include "sdlx/font.h"

class Tooltip;
struct MapDesc;

class MapDetails : public Container {
public: 
	MapDetails(const int w, const int h);

	virtual void render(sdlx::Surface &surface, const int x, const int y) const;
	virtual void get_size(int &w, int &h) const;
	virtual bool onMouse(const int button, const bool pressed, const int x, const int y);
	
	void set(const MapDesc & map_desc);
	~MapDetails();
private: 
	int _w, _h;
	Tooltip *_map_desc, *_ai_hint;
	std::string base, map;
	bool has_tactics;
	
	sdlx::Surface _screenshot, _tactics, _null_screenshot;
	const sdlx::Font *_small_font;
};

#endif

