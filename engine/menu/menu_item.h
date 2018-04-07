#ifndef __BT_MENU_MENUITEM_H__
#define __BT_MENU_MENUITEM_H__

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

/* 
 * Additional rights can be granted beyond the GNU General Public License 
 * on the terms provided in the Exception. If you modify this file, 
 * you may extend this exception to your version of the file, 
 * but you are not obligated to do so. If you do not wish to provide this
 * exception without modification, you must delete this exception statement
 * from your version and license this file solely under the GPL without exception. 
*/

#include "control.h"

namespace sdlx {
	class Font;
}

class MenuItem : public Control {
public:
	MenuItem(const std::string &font, const std::string &area, const std::string &msg);

	virtual void get_size(int&, int&) const;
	virtual void render(sdlx::Surface &surface, const int x, const int y) const;
	virtual bool onKey(const SDL_Keysym sym);
	virtual bool onMouse(const int button, const bool pressed, const int x, const int y);
	inline const std::string get_id() const { return id; }
	
	inline const sdlx::Font * get_font() const { return font; }

private: 
	const sdlx::Font * font;
	std::string id, text;
};

#endif
