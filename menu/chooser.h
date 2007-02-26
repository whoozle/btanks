#ifndef BTANKS_MENU_CHOOSER_H__
#define BTANKS_MENU_CHOOSER_H__

#include "container.h"
#include "sdlx/rect.h"

namespace sdlx {
class Surface;
}

class ImageChooser : public Container {
public: 
	ImageChooser(const std::string &surface, const int n);
	void getSize(int &w, int &h);

	virtual void render(sdlx::Surface &surface, const int x, const int y);
	virtual bool onKey(const SDL_keysym sym);
	virtual bool onMouse(const int button, const bool pressed, const int x, const int y);

private: 
	int _i, _n;
	const sdlx::Surface *_surface, *_left_right;
	int index;
	sdlx::Rect _left_area, _right_area;
};


#endif
