#ifndef BTANKS_MENU_LABEL_H__
#define BTANKS_MENU_LABEL_H__

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

namespace sdlx {
class Surface;
class Font;
}

#include <string>
#include "export_btanks.h"
#include "textual.h"

class BTANKSAPI Label : public TextualControl {
public: 
	Label(const sdlx::Font *font, const std::string &label);
	Label(const std::string &font, const std::string &label);
	virtual void render(sdlx::Surface& surface, const int x, const int y) const;
	virtual void getSize(int &w, int &h) const;

	void set(const std::string &label);
	const std::string get() const;
	
	void setFont(const std::string &font);

private: 
	const sdlx::Font * _font;
	std::string _label;
	int _label_size;
};

#endif

