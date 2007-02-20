#ifndef BTANKS_MENU_SCROLL_LIST_H__
#define BTANKS_MENU_SCROLL_LIST_H__

#include "control.h"
#include "box.h"
#include <list>

class ScrollList : public Control {
public: 
	ScrollList(const int w, const int h);
	virtual void render(sdlx::Surface &surface, const int x, const int y);
	virtual bool onKey(const SDL_keysym sym);
	virtual bool onMouse(const int button, const bool pressed, const int x, const int y);
private:
	Box _background;
};

#endif
