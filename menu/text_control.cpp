#include "text_control.h"
#include "resource_manager.h"
#include "sdlx/font.h"

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
		int c = sym.unicode;
		//LOG_DEBUG(("%d", c));
		if (c >= SDLK_SPACE && c < 128) {
			if (validate(c))
				_text += (char)c;
		}
	}
	}
	return true;
}

void TextControl::render(sdlx::Surface &surface, const int x, const int y) {
	_font->render(surface, x, y, _text.empty()?" ": _text);
}

void TextControl::getSize(int &w, int &h) const {
	h = _font->getHeight();
	w = (_text.empty())?0:_font->render(NULL, 0, 0, _text);
}
