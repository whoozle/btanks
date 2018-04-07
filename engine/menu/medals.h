#ifndef BTANKS_MENU_MEDALS_H__
#define BTANKS_MENU_MEDALS_H__

#include "container.h"

namespace sdlx {
	class Surface;
}

class Campaign;
class Label;
class Tooltip;
class Image;
class Box;

class Medals : public Container {
public: 
	Medals(int w, int h); 
	bool onKey(const SDL_Keysym sym);
	bool onMouse(const int button, const bool pressed, const int x, const int y);
	virtual void hide(const bool hide = true);
	void set(const Campaign * c);
	virtual void render(sdlx::Surface &surface, const int x, const int y) const;
	
	void tick(const float dt);
	
	void left();
	void right();

private: 
	void update();
	void validate();
	void get_medals(const std::string &id, int &now, int &total) const;
	
	int _w, _h;

	const Campaign * campaign;
	std::vector<Image *> tiles;
	int active;

	Box *background;
	Label * title, *numbers;
	Tooltip * hint;
	Image *b_left, *b_right;
	
	float length;
	float dir_x;
};

#endif

