#ifndef BTANKS_MENU_TEXT_CONTROL_H__
#define BTANKS_MENU_TEXT_CONTROL_H__

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

#include "control.h"
#include <string>
#include "alarm.h"
#include "export_btanks.h"

namespace sdlx {
class Font;
}

class BTANKSAPI TextControl : public Control {
public: 
	TextControl(const std::string &font, const unsigned max_len = 0);

	virtual void tick(const float dt);
	void set(const std::string &value);
	const std::string &get() const;
	void get_size(int &w, int &h) const;
	virtual bool onKey(const SDL_keysym sym);
	virtual void render(sdlx::Surface &surface, const int x, const int y) const;

protected:
	virtual const bool validate(const int idx, const int c) const { return true; }

private: 
	void changing() const;

	unsigned _max_len;
	const sdlx::Font *_font; 
	std::string _text;
	Alarm _blink;
	bool _cursor_visible;
	size_t _cursor_position;
};

class BTANKSAPI HostTextControl : public TextControl {
public: 
	HostTextControl(const std::string &font);

protected:
	virtual const bool validate(const int idx, const int c) const;
};

class BTANKSAPI NumericControl : public TextControl {
public: 
	NumericControl(const std::string &font, const int value);

	void set(const int value);
	const int get() const;

protected:
	virtual const bool validate(const int idx, const int c) const;
};

#endif

