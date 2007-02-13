#ifndef __BTANKS_MENU_MAPITEM_H__
#define __BTANKS_MENU_MAPITEM_H__

#include "menuitem.h"
#include <vector>

class MapItem : public MenuItem {
public: 
	MapItem(sdlx::Font &font, const std::string &name);
	virtual void onClick();
	virtual const bool onKey(const SDL_keysym sym);
	void render(sdlx::Surface &dst, const int x, const int y);
	
private:
	void loadScreenshot();
	void updateValue();

	void finish();
	bool _active;
	size_t _index;
	
	std::vector<std::string> _maps;
	sdlx::Surface _screenshot;
};


#endif

