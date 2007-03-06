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
	void getSize(int &w, int &h) const;
	virtual bool onKey(const SDL_keysym sym);
	virtual void render(sdlx::Surface &surface, const int x, const int y);

	virtual const bool validate(const int c) const { return true; }

private: 
	const sdlx::Font *_font; 
	std::string _text, _value;
};

class HostTextControl : public TextControl {
public: 

	HostTextControl(const std::string &font);
	virtual const bool validate(const int c) const;
};

#endif

