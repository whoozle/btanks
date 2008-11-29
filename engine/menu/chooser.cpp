
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
#include "chooser.h"
#include "resource_manager.h"
#include "math/binary.h"
#include "sdlx/surface.h"
#include "sdlx/font.h"
#include "box.h"

#ifdef _WINDOWS
#	define strcasecmp _stricmp
#endif

Chooser::Chooser(const std::string &font, const std::vector<std::string> &options, const std::string &surface, bool with_background) :
_options(options), _i(0), _n(options.size()), _surface(NULL), _w(0), _background(NULL) {
	_disabled.resize(_n);
	if (!surface.empty())
		_surface = ResourceManager->load_surface(surface);
	
	_left_right = ResourceManager->load_surface("menu/left_right.png");
	_font = ResourceManager->loadFont(font, true);
	for(int i = 0; i < _n; ++i) {
		int w = _font->render(NULL, 0, 0, options[i]);
		if (w > _w)
			_w = w;
	}
	if (with_background) {
		int w, h;
		Chooser::get_size(w, h);
		_background = new Box("menu/background_box_dark.png", w, h);
	}
}

const std::string& Chooser::getValue() const {
	if (_options.empty())
		throw_ex(("getValue() on non-text Chooser is invalid"));
	return _options[_i];
}


void Chooser::get_size(int &w, int &h) const {
	if (_n == 0) {
		w = _left_right->get_width();
		h = _left_right->get_height();
		return;
	}
	if (_surface != NULL) {
		w = _left_right->get_width() + _surface->get_width() / _n;
		h = math::max(_left_right->get_height(), _surface->get_height());
	} else {
		w = _left_right->get_width() + _w;
		h = math::max(_left_right->get_height(), _font->get_height());
	}
}

void Chooser::render(sdlx::Surface &surface, const int x, const int y) const {
	if (_background != NULL) 
		_background->render(surface, x - 4, y - 4);
	
	int lrw = _left_right->get_width() / 2;
	int lrh = _left_right->get_height();
	
	int w, h;
	get_size(w, h);
	
	_left_area = sdlx::Rect(0, 0, lrw, lrh);
	_right_area = sdlx::Rect(w - lrw, 0, lrw, lrh);
	
	surface.blit(*_left_right, sdlx::Rect(0, 0, lrw, lrh), x + _left_area.x, y + _left_area.y);
	
	if (_surface) {
		surface.blit(*_surface, 
			sdlx::Rect(_i * _surface->get_width() / _n, 0, _surface->get_width() / _n, _surface->get_height()), 
			x + _left_area.x + lrw, y + (_left_area.h - _surface->get_height())/ 2);
	} else { 
		if (_i < (int)_options.size()) {
			int tw = _font->render(NULL, 0, 0, _options[_i]);
			_font->render(surface, x + _left_area.x + (w - tw) / 2, y + (_left_area.h - _font->get_height())/ 2, _options[_i]);
		}
	} 
	
	surface.blit(*_left_right, sdlx::Rect(lrw, 0, lrw, lrh), x +  _right_area.x, y + _right_area.y);
}

bool Chooser::onMouse(const int button, const bool pressed, const int x, const int y) {
	if (pressed) 
		return true;

	if (_left_area.in(x, y)) {
		left();
		return true;
	} else if (_right_area.in(x, y)) {
		right();
		return true;
	} 
	return false;
}

bool Chooser::onKey(const SDL_keysym sym) {
	switch(sym.sym) {
/*		case SDLK_LEFT: 
			left();
			return true;
		case SDLK_RIGHT: 
			right();
			return true;
*/		default: 
			return false;
	}
}

void Chooser::set(const int i) {
	if (i < 0 || i >= _n)
		throw_ex(("set(%d) is greater than available options (%d)", i, _n));
	_i = i;
	invalidate();
}

void Chooser::set(const std::string &name) {
	for(int i = 0; i < _n; ++i) {
		if (strcasecmp(name.c_str(), _options[i].c_str()) == 0) {
			_i = i;
			invalidate();
			return;
		}
	}
	throw_ex(("chooser doesnt contain option '%s'", name.c_str()));
}

void Chooser::left() {
	if (_n == 0 || _n == 1)
		return;
	do {
		--_i;
		if (_i < 0)
			_i = _n - 1;
	} while(_disabled[_i]);
	invalidate(true);
}

void Chooser::right() {
	if (_n == 0 || _n == 1)
		return;
	do {
		++_i;
		if (_i >= _n)
			_i = 0;
	} while(_disabled[_i]);
	invalidate(true);
}

void Chooser::disable(const int i, const bool value) {
	if (i < 0 || i >= _n)
		throw_ex(("disable(%d) called (n = %d)", i, _n));
	_disabled[i] = value;
	if (_disabled[_i])
		right();
}

Chooser::~Chooser() {
	delete _background;
}
