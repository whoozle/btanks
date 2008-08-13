#ifndef __BTANKS_HUD_H__
#define __BTANKS_HUD_H__

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

#include "sl08/sl08.h"
#include "export_btanks.h"
#include "sdlx/surface.h"
#include "math/v3.h"
#include "alarm.h"

#include <map>
#include <set>
#include <string>
#include <vector>

namespace sdlx {
class Font;
}

class Object;

class BTANKSAPI Hud {
public: 
	Hud(const int w, const int h);
	void toggleMapMode(); 
	
	void render(sdlx::Surface &window) const;

	void renderSplash(sdlx::Surface &window) const;
	const bool renderLoadingBar(sdlx::Surface &window, const float old_progress, const float progress, const char * what, const bool splash = true) const;
	void renderRadar(const float dt, sdlx::Surface &window, const std::vector<v3<int> > &specials, const std::vector<v3<int> > &flags, const sdlx::Rect &viewport);

	void renderStats(sdlx::Surface &surface); //autoproxy
	void renderPlayerStats(sdlx::Surface &surface);
	void renderTeamStats(sdlx::Surface &surface);

	~Hud();
	
private: 

	sl08::slot0<void, Hud> init_map_slot;
	void initMap();
	
	sl08::slot1<void, const std::set<v3<int> > &, Hud> on_destroy_map_slot;
	void on_destroy_map(const std::set<v3<int> > & cells);

	void generateRadarBG(const sdlx::Rect &viewport);

	void renderMod(const Object *obj, sdlx::Surface &window, int &xp, int &yp, const std::string &name, const int icon_w, const int icon_h) const;

	const sdlx::Surface *_background, *_loading_border, *_loading_item, *_splash, *_splitter, *_screen_splitter, *_icons;
	sdlx::Surface _radar_bg, _radar;
	const sdlx::Font *_font, *_big_font, *_small_font;
	const sdlx::Surface *_pointer;
	mutable int _pointer_dir;
	Alarm _update_radar;
	typedef std::map<const std::string, int> IconMap;
	IconMap _icons_map;

	enum MapMode {MapNone, MapSmall, MapFull} _map_mode;
};


#endif

