#ifndef BTANKS_MENU_MAP_DETAILS_H__
#define BTANKS_MENU_MAP_DETAILS_H__

#include "control.h"
#include "box.h"
#include "sdlx/surface.h"
#include "sdlx/font.h"

class Tooltip;

class MapDetails : public Control {
public: 
	MapDetails(const int w, const int h);

	virtual void render(sdlx::Surface &surface, const int x, const int y);
	virtual void getSize(int &w, int &h) const;
	virtual bool onMouse(const int button, const bool pressed, const int x, const int y);
	
	void set(const std::string &base, const std::string &map, const std::string &comment_id);
	~MapDetails();
private: 
	Box _background;
	Tooltip *_map_desc;
	std::string base, map;
	
	sdlx::Surface _screenshot, _tactics, _null_screenshot;
};

#endif

