#ifndef BTANKS_START_SERVER_MENU_H__
#define BTANKS_START_SERVER_MENU_H__

#include "sdlx/rect.h"
#include "base_menu.h"

class MapPicker;
class UpperBox;
class StartServerMenu : public BaseMenu {
public:
	StartServerMenu(const int w, const int h);
	
	void tick(const float dt);
private: 
	int _w, _h;
	UpperBox * _upper_box;
	MapPicker *_map_picker;
};

#endif

