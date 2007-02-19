#ifndef BTANKS_START_SERVER_MENU_H__
#define BTANKS_START_SERVER_MENU_H__

#include "sdlx/rect.h"
#include "base_menu.h"
#include "upper_box.h"

class StartServerMenu : public BaseMenu {
public:
	StartServerMenu(const int w, const int h);
private: 
	int _w, _h;
};

#endif

