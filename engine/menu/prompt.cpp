
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
#include "prompt.h"
#include "text_control.h"
#include "button.h"
#include "sdlx/surface.h"
#include "i18n.h"

Prompt::~Prompt() {
	delete _text;
}

Prompt::Prompt(const int w, const int h, TextControl * text) : _text(text), value(text->get()) {
	_background.init("menu/background_box_dark.png", w, h, 24);
	int mx, my, bw, bh;
	_background.getMargins(mx, my);
	_background.get_size(bw, bh);
	_text_rect = sdlx::Rect(mx, my, w - mx * 2, bh - 2 * my);
	//add(_text_rect, _text = text);

	_b_back = new Button("medium_dark", I18n->get("menu", "back"));
	_b_back->get_size(bw, bh);
	add(w / 4 - bw / 2, h/2, _b_back);

	_b_ok = new Button("medium_dark", I18n->get("menu", "ok"));
	_b_ok->get_size(bw, bh);
	_text_rect.h -= bh;

	add(3 * w / 4 - bw / 2, h/2, _b_ok);
	_modal = true;
}

void Prompt::set(const std::string &value) {
	_text->set(value);
	this->value = value;
}

const std::string &Prompt::get() const {
	return value;
}

void Prompt::tick(const float dt) {
	_text->tick(dt);
	Container::tick(dt);

	if (_b_ok->changed()) {
		_b_ok->reset();
		value = _text->get();
		invalidate();
	} else if (_b_back->changed()) {
		_b_back->reset();
		set(std::string());
		invalidate();
	}
}

bool Prompt::onKey(const SDL_keysym sym) {
	if (sym.sym == SDLK_ESCAPE) {
		//LOG_DEBUG(("invalidating prompt"));
		set(std::string());
		invalidate();
		return true;
	} else if (sym.sym == SDLK_RETURN || sym.sym == SDLK_KP_ENTER) {
		//LOG_DEBUG(("invalidating prompt"));
		value = _text->get();
		invalidate();
		return true;
	} 

	if (_text->onKey(sym) || Container::onKey(sym))
		return true;
	
	return true;
}


void Prompt::get_size(int &w , int &h) const {
	w = _background.w; h = _background.h;
}

bool Prompt::onMouse(const int button, const bool pressed, const int x, const int y) {
	Container::onMouse(button, pressed, x, y);
	return true;
}


#include "mrt/logger.h"

void Prompt::render(sdlx::Surface& surface, const int x, const int y) const {
	_background.render(surface, x, y);
	sdlx::Rect old_clip; 
	surface.get_clip_rect(old_clip);
	
	sdlx::Rect clip  = _text_rect;
	
	clip.x += x;
	clip.y += y;
	
	surface.set_clip_rect(clip);
	int w, h;
	_text->get_size(w, h);
	_text->render(surface, x + _text_rect.x + ((w > _text_rect.w)?(_text_rect.w - w):0), y + _text_rect.y + (_text_rect.h - h) / 2);
	surface.set_clip_rect(old_clip);
	Container::render(surface, x, y);
}
