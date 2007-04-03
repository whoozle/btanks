#ifndef BTANKS_MENU_TEXT_CONTROL_H__
#define BTANKS_MENU_TEXT_CONTROL_H__

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

#include "control.h"
#include <string>
#include "alarm.h"

namespace sdlx {
class Font;
}

class TextControl : public Control {
public: 
	TextControl(const std::string &font);

	virtual void tick(const float dt);
	void set(const std::string &value);
	const std::string &get() const;
	void getSize(int &w, int &h) const;
	virtual bool onKey(const SDL_keysym sym);
	virtual void render(sdlx::Surface &surface, const int x, const int y);

	virtual const bool validate(const int c) const { return true; }

private: 
	const sdlx::Font *_font; 
	std::string _text, _value;
	Alarm _blink;
	bool _cursor_visible;
	size_t _cursor_position;
};

class HostTextControl : public TextControl {
public: 

	HostTextControl(const std::string &font);
	virtual const bool validate(const int c) const;
};

#endif

