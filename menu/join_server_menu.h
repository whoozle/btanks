#ifndef BTANKS_JOIN_SERVER_MENU_H__
#define BTANKS_JOIN_SERVER_MENU_H__

#include "sdlx/rect.h"
#include "base_menu.h"
#include "upper_box.h"

class Button;
class MainMenu;
class HostList;
class MapDetails;
class Prompt;
class Chooser;

class JoinServerMenu : public BaseMenu {
public:
	JoinServerMenu(MainMenu *parent, const int w, const int h);
	~JoinServerMenu();

	void tick(const float dt);
	void join();
	
	virtual bool onKey(const SDL_keysym sym);

private: 
	MainMenu *_parent;
	UpperBox *_upper_box;
	HostList *_hosts;
	MapDetails *_details;
	Prompt *_add_dialog;
	Chooser *_vehicle;
	Button *_back, *_add, *_scan, *_join;
};

#endif

