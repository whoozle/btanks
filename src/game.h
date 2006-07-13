#ifndef __BT_GAME_H__
#define __BT_GAME_H__

#include "sdlx/surface.h"
#include "mrt/singleton.h"
#include <string>
#include <vector>
#include <sigc++/sigc++.h>

#include "menu.h"
#include "map.h"

class Player;

class IGame {
public: 
	DECLARE_SINGLETON(IGame);

	static const std::string data_dir;
	//signals
	sigc::signal2<void, const Uint8, const SDL_keysym> key_signal;

	void init(const int argv, const char **argc);
	void run();
	void deinit();


	IGame();
	~IGame();
private:
	void onKey(const Uint8 type, const SDL_keysym sym);
	void onMenu(const std::string &name);

	bool _running;
	sdlx::Surface _window;

	MainMenu _main_menu;
	Map _map;
};

SINGLETON(Game, IGame);

#endif

