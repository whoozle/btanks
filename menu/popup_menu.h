#ifndef __BTANKS_POPUP_MENU_H__
#define __BTANKS_POPUP_MENU_H__

#include "container.h"
#include "export_btanks.h"
#include <string>
#include <set>

class Box;

class BTANKSAPI PopupMenu : public Container {
public: 
	PopupMenu();
	~PopupMenu();
	
	void append(const std::string &item, const bool state);
	void clear();
	void get(std::set<std::string> &labels) const;

	virtual bool onMouse(const int button, const bool pressed, const int x, const int y);
	virtual bool onMouseMotion(const int state, const int x, const int y, const int xrel, const int yrel);
	virtual void render(sdlx::Surface &surface, const int x, const int y);

private: 
	PopupMenu(const PopupMenu& );
	const PopupMenu & operator=(const PopupMenu& );
	Box * _background;
	std::string result;
	v2<int> hl_pos;
};

#endif

