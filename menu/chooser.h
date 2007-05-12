#ifndef BTANKS_MENU_CHOOSER_H__
#define BTANKS_MENU_CHOOSER_H__

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
#include "sdlx/rect.h"
#include <string>
#include <vector>
#include "export_btanks.h"

namespace sdlx {
class Surface;
class Font;
}

class BTANKSAPI Chooser : public Container {
public: 
	Chooser(const std::string &font, const std::string &label, const std::vector<std::string> &options, const std::string &surface = std::string());
	void getSize(int &w, int &h) const;

	virtual void render(sdlx::Surface &surface, const int x, const int y);
	virtual bool onMouse(const int button, const bool pressed, const int x, const int y);

	void set(const int i);
	const int get() const { return _i; }
	const int size() const { return _n; }
	void set(const std::string &name);
	const std::string& getValue() const;
	
	void left();
	void right();
	
	void disable(const int i, const bool value = true);

private: 
	std::vector<std::string> _options;
	std::vector<bool> _disabled;
	int _i, _n;
	const sdlx::Surface *_surface, *_left_right;

	//textual chooser: 
	const sdlx::Font *_font;
	int _w;

	sdlx::Rect _left_area, _right_area;
};


#endif
