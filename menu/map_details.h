#ifndef BTANKS_MENU_MAP_DETAILS_H__
#define BTANKS_MENU_MAP_DETAILS_H__

#include "control.h"
#include "box.h"
#include "sdlx/surface.h"
#include "sdlx/font.h"

class MapDetails : public Control {
public: 
	MapDetails(const int w, const int h);

	virtual void render(sdlx::Surface &surface, const int x, const int y);
	virtual bool onKey(const SDL_keysym sym);
	virtual bool onMouse(const int button, const bool pressed, const int x, const int y);
	
	void set(const std::string &base, const std::string &map, const std::string &comments);
private: 
	Box _background;
	
	sdlx::Surface _screenshot, _null_screenshot;
	const sdlx::Font *_font;
	std::string _comments;
};

#endif

