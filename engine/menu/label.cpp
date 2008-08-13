
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
#include "label.h"
#include "sdlx/font.h"
#include "sdlx/surface.h"
#include "sdlx/rect.h"
#include "resource_manager.h"

#define LABEL_SPEED (30)

Label::Label(const sdlx::Font *font, const std::string &label) : 
	_font(font), _label(label), _label_size(_font->render(0, 0, 0, _label)), 
	_max_width(0), _max_height(0),  x_pos(0), x_vel(LABEL_SPEED) {}

Label::Label(const std::string &font, const std::string &label) : 
	_font(ResourceManager->loadFont(font, true)), _label(label), _label_size(_font->render(0, 0, 0, _label)), 
	_max_width(0), _max_height(0), x_pos(0), x_vel(LABEL_SPEED) {}

void Label::get_size(int &w, int &h) const {
	w = _max_width <= 0? _label_size: (_label_size < _max_width? _label_size: _max_width);
	h = _font->get_height();
}

void Label::setFont(const std::string &font) {
	_font = ResourceManager->loadFont(font, true);
	_label_size = _font->render(0, 0, 0, _label);
}

void Label::set(const std::string &label) {
	_label = label;
	_label_size = _font->render(0, 0, 0, _label);
}

const std::string Label::get() const { 
	return _label; 
}

void Label::render(sdlx::Surface& surface, int x, int y) const {
	if (_max_width <= 0) {
		_font->render(surface, x, y, _label);
		return;
	} 
	//smooth scrolling
	sdlx::Rect clip;
	surface.get_clip_rect(clip);
	surface.set_clip_rect(sdlx::Rect(x, y, _max_width, _max_height));
	_font->render(surface, x - (int)x_pos, y, _label);
	surface.set_clip_rect(clip);
}

void Label::set_size(const int w, const int h) {
	LOG_DEBUG(("setting maximum size %dx%d", w, h));
	_max_width= w;
	_max_height = h;
}

#define MARGIN 10
#define OVERLAP 4

void Label::tick(const float dt) {
	TextualControl::tick(dt);
	if (_max_width <= 0)
		return;
	if (_label_size <= _max_width) {
		x_pos = 0;
		return;
	}
	int delta =  _label_size - _max_width;
	//LOG_DEBUG(("%d",delta));
	float m = delta >= MARGIN? 1.0f: 1.0f * (delta + MARGIN / 2) / (MARGIN + MARGIN / 2);
	
	x_pos += x_vel * dt * m;
	if (x_pos + _max_width - OVERLAP > _label_size) {
		x_pos = _label_size - _max_width + OVERLAP;
		x_vel = -LABEL_SPEED;
	}
	if (x_pos < - OVERLAP) {
		x_pos = -OVERLAP;
		x_vel = LABEL_SPEED;
	}
}
