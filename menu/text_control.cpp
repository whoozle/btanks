#include "text_control.h"
#include "resource_manager.h"
#include "sdlx/font.h"

TextControl::TextControl(const std::string &font) {
	_font = ResourceManager->loadFont(font, true);
}

void TextControl::set(const std::string &value) {
	_value = _text = value;
}
const std::string& TextControl::get() const {
	return _text;
}


bool TextControl::onKey(const SDL_keysym sym) {
	switch(sym.sym) {
	case SDLK_ESCAPE: 
		_text = _value;
		_changed = true;
		break;

	case SDLK_RETURN: 
		_value = _text;
		_changed = true;
		break;
		
	case SDLK_BACKSPACE:
		if (!_text.empty()) 
			_text = _text.substr(0, _text.size() - 1);
		break;

	case SDLK_DELETE:
		_text.clear();
		break;
		
	default: {
		int c = sym.sym;
		if (c >= SDLK_SPACE && c < 128) {
			_text += (char)c;
		}
	}
	}
	return true;
}

void TextControl::render(sdlx::Surface &surface, const int x, const int y) {
	_font->render(surface, x, y, _text.empty()?" ": _text);
}
