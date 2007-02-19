#ifndef __MENU_UPPER_BOX_H__
#define __MENU_UPPER_BOX_H__

#include <string>
#include "box.h"
#include "control.h"
#include "sdlx/font.h"
#include "sdlx/rect.h"

class UpperBox : public Control, public Box {
public: 
	std::string value;

	virtual void init(int w, int h, const bool server);
	virtual void render(sdlx::Surface &surface, const int x, const int y);
	virtual bool onKey(const SDL_keysym sym);
	virtual bool onMouse(const int button, const bool pressed, const int x, const int y);
private: 
	bool _server;	
	const sdlx::Surface *_checkbox;
	sdlx::Font _big, _medium;
	sdlx::Rect _on_area, _off_area;
};

#endif
