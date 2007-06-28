#ifndef __BTANKS_POPUP_MENU_H__
#define __BTANKS_POPUP_MENU_H__

#include "container.h"
#include "export_btanks.h"
#include <string>

class Box;

class BTANKSAPI PopupMenu : public Container {
public: 
	PopupMenu();
	
	void append(const std::string &item);

	virtual bool onMouse(const int button, const bool pressed, const int x, const int y);
	virtual bool onMouseMotion(const int state, const int x, const int y, const int xrel, const int yrel);
private: 
	Box * _background;
	std::string result;
};

#endif

