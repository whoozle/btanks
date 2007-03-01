#ifndef BTANKS_START_SERVER_MENU_H__
#define BTANKS_START_SERVER_MENU_H__

#include "sdlx/rect.h"
#include "base_menu.h"

class MapPicker;
class Button;
class MainMenu;

class StartServerMenu : public BaseMenu {
public:
	StartServerMenu(MainMenu *parent, const int w, const int h);
	~StartServerMenu();

	void start();	
	void tick(const float dt);
	virtual bool onKey(const SDL_keysym sym);
	
private: 
	MainMenu *_parent;
	MapPicker *_map_picker;
	Button *_back, *_start;
};

#endif

