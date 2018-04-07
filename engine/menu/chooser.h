#ifndef BTANKS_MENU_CHOOSER_H__
#define BTANKS_MENU_CHOOSER_H__

/* Battle Tanks Game
 * Copyright (C) 2006-2009 Battle Tanks team
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
#include "sdlx/rect.h"
#include <string>
#include <vector>
#include "export_btanks.h"

namespace sdlx {
class Surface;
class Font;
}

class Box;

class BTANKSAPI Chooser : public Control {
public: 
	Chooser(const std::string &font, const std::vector<std::string> &options, const std::string &surface = std::string(), bool with_background = false);
	~Chooser();
	void get_size(int &w, int &h) const;

	virtual void render(sdlx::Surface &surface, const int x, const int y) const;
	virtual bool onMouse(const int button, const bool pressed, const int x, const int y);
	virtual bool onKey(const SDL_Keysym sym);

	void set(const int i);
	const int get() const { return _i; }
	const int size() const { return _n; }
	bool empty() const { return _options.empty(); }
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
	Box * _background;

	mutable sdlx::Rect _left_area, _right_area;
};


#endif
