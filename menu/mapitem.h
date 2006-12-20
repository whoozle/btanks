#ifndef __BTANKS_MENU_MAPITEM_H__
#define __BTANKS_MENU_MAPITEM_H__

#include "menuitem.h"

class MapItem : public MenuItem {
public: 
	MapItem(sdlx::TTF &font, const std::string &name);
	virtual void onClick();
	virtual const bool onKey(const Uint8 type, const SDL_keysym sym);
private:
	void finish();
	bool _active;
};


#endif

