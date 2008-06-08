#ifndef BTANKS_NOTEPAD_H__
#define BTANKS_NOTEPAD_H__

#include "sdlx/rect.h"
#include <string>
#include <vector>
#include "control.h"

namespace sdlx {
	class Surface;
	class Font;
}

//this notepad is actually a fake. 
//it's not a container, just a clickable tab bar :) sorry :)

class Notepad : public Control {
public: 
	Notepad(const int w, const std::string &font);
	void add(const std::string &area, const std::string &label);
	void render(sdlx::Surface &surface, const int x, const int y) const;
	void get_size(int &w, int &h) const;
	bool onMouse(const int button, const bool pressed, const int x, const int y);
	bool onKey(const SDL_keysym sym);
	void set(const int idx);
	int get() const { return (int)current_page; }
	
	void left();
	void right();

private:
	void recalculate_sizes();
	
	int tab_x1, tab_x2, tab_w, width;
	sdlx::Rect tab_left, tab_right, tab_bg;
	
	const sdlx::Surface * tabbg;
	const sdlx::Font * font;
	
	struct Page {
		std::string label;
		sdlx::Rect tab_rect;
		//aligning ? )
	};

	size_t current_page;
	
	std::vector<Page> pages;
};

#endif

