#ifndef __BTANKS_MENU_VEHICLE_ITEM_H__
#define __BTANKS_MENU_VEHICLE_ITEM_H__

#include "menuitem.h"
#include <vector>

class VehicleItem : public MenuItem {
public: 
	VehicleItem(const sdlx::Font *font, const std::string &name, const std::string &subkey);
	virtual void onClick();
	virtual const bool onKey(const SDL_keysym sym);
	void render(sdlx::Surface &dst, const int x, const int y);
	
private:
	void updateValue();

	void finish();
	bool _active;
	size_t _index;
	
	std::vector<std::string> _items;
	std::string _subkey;
};


#endif

