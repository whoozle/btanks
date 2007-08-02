#ifndef __MENU_UPPER_BOX_H__
#define __MENU_UPPER_BOX_H__

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

#include <string>
#include "container.h"
#include "sdlx/rect.h"

namespace sdlx {
	class Font;
}

class PlayerNameControl;

class UpperBox : public Container {
public: 
	std::string value;

	UpperBox(int w, int h, const bool server);
	virtual void render(sdlx::Surface &surface, const int x, const int y);
	virtual bool onMouse(const int button, const bool pressed, const int x, const int y);
private: 
	bool _server;	
	const sdlx::Surface *_checkbox;
	const sdlx::Font *_big, *_medium;
	sdlx::Rect _on_area, _off_area;
	
	PlayerNameControl *_player1_name, *_player2_name;
};

#endif
