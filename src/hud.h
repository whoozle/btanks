#ifndef __BTANKS_HUD_H__
#define __BTANKS_HUD_H__

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

#include "sdlx/surface.h"
#include "sdlx/font.h"
#include "alarm.h"
#include <map>
#include <string>

class Font;
class Object;

class Hud {
public: 
	Hud(const int w, const int h);
	void render(sdlx::Surface &window) const;

	void renderSplash(sdlx::Surface &window) const;
	const bool renderLoadingBar(sdlx::Surface &window, const float old_progress, const float progress) const;
	void renderRadar(const float dt, sdlx::Surface &window);

	~Hud();

private: 

	void renderMod(const Object *obj, sdlx::Surface &window, int &xp, int &yp, const std::string &name, const int icon_w, const int icon_h) const;

	sdlx::Surface _background, _loading_border, _loading_item, _splash, _radar, _splitter;
	sdlx::Font _font;
	Alarm _update_radar;
	typedef std::map<const std::string, int> IconMap;
	IconMap _icons_map;
	sdlx::Surface _icons;
};


#endif

