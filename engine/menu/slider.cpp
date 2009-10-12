
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
#include "slider.h"
#include "sdlx/surface.h"
#include "resource_manager.h"
#include "window.h"
#include "sdlx/sdlx.h"
#include "math/unary.h"

Slider::Slider(const float value) : _n(10), _value(value), _grab(false) {
	if (value > 1.0f) 
		throw_ex(("slider accepts only values between 0 and 1 (inclusive)"));
	_tiles = ResourceManager->load_surface("menu/slider.png");

	mm_slot.assign(this, &Slider::onMouseMotion, Window->mouse_motion_signal);
}


void Slider::render(sdlx::Surface &surface, const int x, const int y) const {
	int w = _tiles->get_width() / 2, h = _tiles->get_height();
	sdlx::Rect bound(0, 0, w, h), pointer(w, 0, w, h);
	for(int i = 0; i < _n; ++i) 
		surface.blit(*_tiles, bound, w / 2 + x + i * w, y);
	int xp = x + (int)(_value * _n * w);
	surface.blit(*_tiles, pointer, xp, y);
}

void Slider::get_size(int &w, int &h) const {
	w = (_tiles->get_width() / 2) * (_n + 1);
	h = _tiles->get_height();
}

bool Slider::onMouse(const int button, const bool pressed, const int x, const int y) {
	if (!pressed && _grab) {
		_grab = false;
		return true;
	}
	if (pressed && !_grab) {
		int w = _tiles->get_width() / 2;
		int xp = (int)(_value * _n * w + w/2);
		if (math::abs(x - xp) < w / 2) {
			_grab = true;
			_grab_state = SDL_GetMouseState(NULL, NULL);
		} else {
			int dir = math::sign(x - xp);
			_value += dir / (float)_n;
			validate();
			invalidate();
		}
	}	
	return false;
}

bool Slider::onMouseMotion(const int state, const int x, const int y, const int xrel, const int yrel) {
	if (!_grab) 	
		return false;
	if (state != _grab_state) {
		_grab = false;
		return true;
	}
	int w = _tiles->get_width() / 2;
	_value += 1.0f * xrel / w / _n;
	validate();
	invalidate();
	//LOG_DEBUG(("tracking mouse: %d %d %d %d", x, y, xrel, yrel));
	
	return true;
}

void Slider::set(const float value) {
	_value = value;
	validate();
	invalidate();
}

void Slider::validate() {
	if (_value < 0)
		_value = 0;
	else if (_value > 1)
		_value = 1;
}
