#ifndef BTANKS_JOIN_SERVER_MENU_H__
#define BTANKS_JOIN_SERVER_MENU_H__

#include "sdlx/rect.h"
#include "base_menu.h"

class Button;
class MainMenu;

class JoinServerMenu : public BaseMenu {
public:
	JoinServerMenu(MainMenu *parent, const int w, const int h);
	~JoinServerMenu();

	void tick(const float dt);
	void join();
	
	virtual bool onKey(const SDL_keysym sym);

private: 
	MainMenu *_parent;
	Button *_back, *_scan, *_join;
};

#endif

