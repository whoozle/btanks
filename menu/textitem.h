#ifndef __BTANKS_MENU_TEXTITEM_H__
#define __BTANKS_MENU_TEXTITEM_H__

#include "menuitem.h"

class TextItem : public MenuItem {
public: 
	TextItem(sdlx::Font &font, const std::string &config_name, const std::string &name, const std::string &value = std::string());
	virtual void onClick();
	virtual const bool onKey(const SDL_keysym sym);
private:
	std::string _config_name;
	void finish();
	bool _active;
	sdlx::Color _old_bg;
};


#endif

