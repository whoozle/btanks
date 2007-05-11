#ifndef BTANKS_MENU_SCROLL_LIST_H__
#define BTANKS_MENU_SCROLL_LIST_H__

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
#include "box.h"
#include <deque>
#include "sdlx/font.h"
#include "sdlx/rect.h"
#include "export_btanks.h"

class BTANKSAPI ScrollList : public Control {
public: 
	ScrollList(const std::string &font, const int w, const int h);
	
	virtual void clear();
	virtual void add(const std::string &item);
	virtual void add(Control *control);
	
	const int get() const { return _current_item; }
	const std::string getValue() const;
	void set(const int idx) { _current_item = idx; }
	void remove(const int idx);
	
	virtual void tick(const float dt);
	virtual void render(sdlx::Surface &surface, const int x, const int y);
	virtual bool onKey(const SDL_keysym sym);
	virtual bool onMouse(const int button, const bool pressed, const int x, const int y);
	virtual void getSize(int &w, int &h) const;

	const int getItemY(const int idx) const;
	const int getItemIndex(const int yp) const;

	~ScrollList();
private:
	Box _background;
	const sdlx::Surface *_scrollers;
	sdlx::Rect _up_area, _down_area, _items_area;
	int _client_w, _client_h;


	float _pos, _vel;
protected:
	const sdlx::Font *_font;
	typedef std::deque<Control *> List;
	List _list;
	int _current_item;
};

#endif
