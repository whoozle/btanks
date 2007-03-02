#ifndef BTANKS_MENU_TEXT_CONTROL_H__
#define BTANKS_MENU_TEXT_CONTROL_H__

#include "control.h"
#include <string>

namespace sdlx {
class Font;
}

class TextControl : public Control {
public: 
	TextControl(const std::string &font);
	void set(const std::string &value);
	const std::string &get() const;
	virtual bool onKey(const SDL_keysym sym);
	virtual void render(sdlx::Surface &surface, const int x, const int y);

private: 
	const sdlx::Font *_font; 
	std::string _text, _value;
};

#endif

