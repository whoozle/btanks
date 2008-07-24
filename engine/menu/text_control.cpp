
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
#include "text_control.h"
#include "resource_manager.h"
#include "sdlx/font.h"
#include "config.h"
#include "sound/mixer.h"
#include "mrt/utf8_utils.h"
#include <stdlib.h>
#include <ctype.h>

void TextControl::changing() const {
	Mixer->playSample(NULL, "menu/change.ogg", false);
}

NumericControl::NumericControl(const std::string &font, const int value) : TextControl(font, 8) {
	set(value);
}

const bool NumericControl::validate(const int idx, const int c) const {
	if (idx == 0 && (c == '+' || c == '-'))
		return true;
	
	return (c >= '0' && c <= '9');
}

void NumericControl::set(const int value) {
	TextControl::set(mrt::format_string("%d", value));
}

const int NumericControl::get() const {
	const std::string &value = TextControl::get();
	if (value.empty())
		return 0;
	return atoi(value.c_str());
}


HostTextControl::HostTextControl(const std::string &font) : TextControl(font, 255) {}

const bool HostTextControl::validate(const int idx, const int c) const {
	if (c >= 'a' && c <= 'z')
		return true;

	if (c >= '0' && c <= '9')
		return true;

	if (idx != 0 && (c == '.' || c == '-' || c == ':')) 
		return true;

	return false;
}

/////////////////////////////////////////////////////////////

TextControl::TextControl(const std::string &font, unsigned max_len) : 
_max_len(max_len), _blink(true), _cursor_visible(true), _cursor_position(0) {
	_font = ResourceManager->loadFont(font, true);
	GET_CONFIG_VALUE("menu.cursor-blinking-interval", float, cbi, 0.4f);
	_blink.set(cbi);
}

void TextControl::set(const std::string &value) {
	_text = value;
	_cursor_position = _text.size();
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
	case SDLK_HOME: 
		_cursor_position = 0;
		break;
		
	case SDLK_END:
		_cursor_position = _text.size();
		break;

	case SDLK_LEFT: 
		_cursor_position = mrt::utf8_left(_text, _cursor_position);
		break;

	case SDLK_RIGHT: 
		_cursor_position = mrt::utf8_right(_text, _cursor_position);
		break;
		
	case SDLK_BACKSPACE:
		if (sym.mod & KMOD_CTRL) {
			//set(std::string());
			size_t next = _cursor_position; 
			while(next > 0) {
				next = mrt::utf8_left(_text, next);
				if (_text[next] & 0x80) 
					continue;
				if (!isalnum(_text[next]))
					break;
			}
			_text.erase(next, _cursor_position - next);
			_cursor_position = next;
			
			break;
		}
		if (!_text.empty() && _cursor_position > 0) {
			_cursor_position = mrt::utf8_backspace(_text, _cursor_position);
		}
		break;

	case SDLK_DELETE: {
		if (_cursor_position >= _text.size())
			break;
		size_t r = mrt::utf8_right(_text, _cursor_position);
		mrt::utf8_backspace(_text, r);
		}
		break;
		
	default: {
		int c = sym.unicode;
		//LOG_DEBUG(("%d", c));
		if (c >= SDLK_SPACE) {
			if (_max_len && mrt::utf8_length(_text) >= _max_len)
				return true;
			
			if (validate(_cursor_position, c)) {
				if (_cursor_position >= _text.size()) {
					mrt::utf8_add_wchar(_text, c);
					_cursor_position = _text.size();
				} else {
					std::string chr;
					mrt::utf8_add_wchar(chr, c);
					_text.insert(_cursor_position, chr);
					_cursor_position += chr.size();
				}
				return true;
			}
		}
		return false;
	}
	}
	changing();
	return true;
}

void TextControl::render(sdlx::Surface &surface, const int x, const int y) const {
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

void TextControl::get_size(int &w, int &h) const {
	h = _font->get_height();
	w = (_text.empty())?0:_font->render(NULL, 0, 0, _text);
}
