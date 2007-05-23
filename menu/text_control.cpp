
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
#include "text_control.h"
#include "resource_manager.h"
#include "sdlx/font.h"
#include "config.h"
#include "sound/mixer.h"

void TextControl::changing() const {
	Mixer->playSample(NULL, "menu/change.ogg", false);
}


HostTextControl::HostTextControl(const std::string &font) : TextControl(font) {}

const bool HostTextControl::validate(const int c) const {
	if (c >= 'a' && c <= 'z')
		return true;

	if (c >= '0' && c <= '9')
		return true;

	if (c == '.' || c == '-') 
		return true;

	return false;
}

/////////////////////////////////////////////////////////////

TextControl::TextControl(const std::string &font) : _blink(true), _cursor_visible(false), _cursor_position(0) {
	_font = ResourceManager->loadFont(font, true);
	GET_CONFIG_VALUE("menu.cursor-blinking-interval", float, cbi, 0.4);
	_blink.set(cbi);
}

void TextControl::set(const std::string &value) {
	_value = _text = value;
	_cursor_position = value.size();
}
const std::string& TextControl::get() const {
	return _text;
}

void TextControl::tick(const float dt) {
	if (_blink.tick(dt))
		_cursor_visible = !_cursor_visible;
	Control::tick(dt);
}


bool TextControl::onKey(const SDL_keysym sym) {
	switch(sym.sym) {
	case SDLK_ESCAPE: 
		_text = _value;
		invalidate();
		break;

	case SDLK_RETURN: 
		_value = _text;
		invalidate();
		break;

	case SDLK_LEFT: 
		if (_cursor_position > 0) 
			--_cursor_position;
		break;

	case SDLK_RIGHT: 
		if (_cursor_position < _text.size()) 
			++_cursor_position;
		break;
		
	case SDLK_BACKSPACE:
		if (!_text.empty() && _cursor_position > 0) {
			_text = _text.erase(--_cursor_position, 1);
		}
		break;

	case SDLK_DELETE:
		if (_cursor_position < _text.size())
			_text.erase(_cursor_position, 1);
		break;
		
	default: {
		int c = sym.unicode;
		//LOG_DEBUG(("%d", c));
		if (c >= SDLK_SPACE && c < 128) {
			if (validate(c)) {
				_text.insert(_cursor_position, 1, (char)c);
				_cursor_position += 1;
			}
		}
	}
	}
	changing();
	return true;
}

void TextControl::render(sdlx::Surface &surface, const int x, const int y) {
	int xp = x;
	if (!_text.empty())
		xp += _font->render(surface, xp, y, _text.substr(0, _cursor_position));
	
	int xc = xp; 
	int cw = 0, curw = 0;
	if (_cursor_visible && _cursor_position < _text.size()) {
		cw = _font->render(NULL, 0, 0, std::string(&_text[_cursor_position], 1));
		curw = _font->render(NULL, 0, 0, "_");
	}
	
	if (!_text.empty() && _cursor_position < _text.size())
		_font->render(surface, xp, y, _text.substr(_cursor_position));

	if (_cursor_visible) {
		_font->render(surface, xc + (cw - curw) / 2 , y+ 4, "_");
	}
}

void TextControl::getSize(int &w, int &h) const {
	h = _font->getHeight();
	w = (_text.empty())?0:_font->render(NULL, 0, 0, _text);
}
