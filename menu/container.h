#ifndef BTANKS_MENU_CONTAINER_H__
#define BTANKS_MENU_CONTAINER_H__

#include "control.h"
#include <list>
#include <math/v2.h>

class Container : public Control {
public: 
	Container() {}
	virtual void tick(const float dt);
	virtual void render(sdlx::Surface &surface, const int x, const int y);
	virtual void getSize(int &w, int &h) const;
	virtual bool onKey(const SDL_keysym sym);
	virtual bool onMouse(const int button, const bool pressed, const int x, const int y);

	void add(const int x, const int y, Control *ctrl);
	void clear();
	
	~Container();
private: 
	Container(const Container &);
	const Container& operator=(const Container &);

protected:
	typedef std::list<std::pair<v2<int>, Control *> > ControlList;
	ControlList _controls;
};

#endif

