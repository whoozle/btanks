
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
#include "textitem.h"
#include "mrt/logger.h"
#include <ctype.h>
#include "config.h"

TextItem::TextItem(const sdlx::Font *font, const std::string &config_name, const std::string &name, const std::string &value): 
MenuItem(font, name, "text", value, value), _config_name(config_name), _active(false)
{}

void TextItem::onClick() {
	_active = true;
	render();
}

void TextItem::finish() {
	render();
	_active = false;
}


const bool TextItem::onKey(const SDL_keysym sym) {
	if (!_active)
		return false;

	switch(sym.sym) {
	case SDLK_ESCAPE: 
		_text = _value;
		finish();
		break;

	case SDLK_RETURN: 
		_value = _text;
		Config->set(_config_name, _value);
		finish();
		break;
		
	case SDLK_BACKSPACE:
		if (!_text.empty()) 
			_text = _text.substr(0, _text.size() - 1);
		render();
		break;

	case SDLK_DELETE:
		_text.clear();
		render();
		break;
		
	default: {
		int c = sym.sym;
		if (c >= SDLK_SPACE && c < 128) {
			c = toupper(c);
			_text += (char)c;
			render();
		}
	}
	}
	
	//LOG_DEBUG(("onKey %u %u", type, sym.sym));

	return true;
}
