#ifndef BTANKS_MENU_CONTAINER_H__
#define BTANKS_MENU_CONTAINER_H__

#include "control.h"
#include <list>
#include "sdlx/rect.h"

class Control;

class Container : public Control {
public: 
	virtual void render(sdlx::Surface &surface, const int x, const int y);
	virtual bool onKey(const SDL_keysym sym);
	virtual bool onMouse(const int button, const bool pressed, const int x, const int y);

	void add(const sdlx::Rect &r, Control *ctrl);
	~Container();
private: 
	typedef std::list<std::pair<sdlx::Rect, Control *> > ControlList;
	ControlList _controls;
};

#endif

