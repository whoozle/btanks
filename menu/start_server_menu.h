#ifndef BTANKS_START_SERVER_MENU_H__
#define BTANKS_START_SERVER_MENU_H__

#include "base_menu.h"
#include "upper_box.h"

class StartServerMenu : public BaseMenu {
public:
	StartServerMenu();
	virtual void render(sdlx::Surface &dst);
	virtual bool onKey(const SDL_keysym sym);
private: 
	UpperBox _upper_box;
};

#endif

