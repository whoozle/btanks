#ifndef __BT_MENUITEM_H__
#define __BT_MENUITEM_H__

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

#include "sdlx/color.h"
#include "sdlx/surface.h"
#include <string>
#include <map>

namespace sdlx {
class TTF;
}

class MenuItem {
public:
	const std::string name;
	const std::string type;
	
	MenuItem(sdlx::TTF &font, const std::string &name, const std::string &type, const std::string &text, const std::string &value = std::string());
	void render(sdlx::Surface &dst, const int x, const int y, const bool inverse);
	void getSize(int &w, int &h) const;

	virtual void onClick() {}
	virtual const std::string getValue() const;
	virtual ~MenuItem() {}

private:
	void render(sdlx::TTF &);
	sdlx::TTF & _font;
		
	sdlx::Color _color;
	sdlx::Surface _normal, _inversed;
	std::string _text, _value;
};

/*
class ChoiceItem : public MenuItem {
public:
	typedef std::map<int, std::string> Choices;
	
	ChoiceItem(sdlx::TTF &font, const std::string &name, const std::string &type, const Choices &choices);
private:
};
*/
#endif
