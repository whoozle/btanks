#include "textitem.h"
#include "mrt/logger.h"
#include <ctype.h>
#include "config.h"

TextItem::TextItem(sdlx::Font &font, const std::string &config_name, const std::string &name, const std::string &value): 
MenuItem(font, name, "text", value, value), _config_name(config_name), _active(false), _old_bg(0,0,0)
{}

void TextItem::onClick() {
	_active = true;
	_old_bg = _bgcolor;
	_bgcolor = sdlx::Color(128, 0, 0);
	render();
}

void TextItem::finish() {
	_bgcolor = _old_bg;
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
