#ifndef BTANKS_MENU_SCROLL_LIST_H__
#define BTANKS_MENU_SCROLL_LIST_H__

#include "control.h"
#include "box.h"
#include <deque>
#include "sdlx/font.h"

class ScrollList : public Control {
public: 
	ScrollList(const int w, const int h);
	void add(const std::string &item) { _list.push_back(item); }
	
	virtual void render(sdlx::Surface &surface, const int x, const int y);
	virtual bool onKey(const SDL_keysym sym);
	virtual bool onMouse(const int button, const bool pressed, const int x, const int y);
private:
	Box _background;
	sdlx::Font _font;

	typedef std::deque<std::string> List;
	List _list;

	int _pos;
};

#endif
