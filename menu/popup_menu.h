#ifndef __BTANKS_TOGGLE_MENU_H__
#define __BTANKS_TOGGLE_MENU_H__

#include "export_btanks.h"
#include "container.h"

class Box;

class BTANKSAPI PopupMenu : public Container {
public: 
	PopupMenu();
	
	void append(const std::string &item);
	
	virtual bool onKey(const SDL_keysym sym);
	virtual bool onMouse(const int button, const bool pressed, const int x, const int y);
	virtual bool onMouseMotion(const int state, const int x, const int y, const int xrel, const int yrel);

};

#endif

