#ifndef __BT_GAME_H__
#define __BT_GAME_H__

/* Battle Tanks Game
 * Copyright (C) 2006-2008 Battle Tanks team
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

/* 
 * Additional rights can be granted beyond the GNU General Public License 
 * on the terms provided in the Exception. If you modify this file, 
 * you may extend this exception to your version of the file, 
 * but you are not obligated to do so. If you do not wish to provide this
 * exception without modification, you must delete this exception statement
 * from your version and license this file solely under the GPL without exception. 
*/

#include "mrt/singleton.h"
#include <string>
#include <map>
#include <vector>
#include <set>

#include "math/v2.h"
#include "player_state.h"
#include "alarm.h"

#include "export_btanks.h"
#include "sdlx/sdlx.h"
#include "sl08/sl08.h"

#include "random_pool.h"

class BaseObject;
class Object;
class Message;
class Server;
class Client;
class Connection;
class ControlMethod;
class PlayerSlot;
class Hud;
class Cheater;
class MainMenu;
class Tooltip;
class Chat;

namespace sdlx {
	class Surface;
	class Rect;
	class Font;
}

struct Cutscene {
	virtual void render(const float dt, sdlx::Surface &surface) = 0;
	virtual ~Cutscene() {}
};

class BTANKSAPI IGame {

public: 
	DECLARE_SINGLETON(IGame);

	void init(const int argc, char *argv[]);
	void run();
	void deinit();
	
	void clear();
	void pause();

	IGame();
	~IGame();
	
	//stupid visual effect
	void shake(const float duration, const int intensity);
	
	static void loadPlugins();
	
	Chat *getChat() { return _net_talk; }
	
	void stop();
	
	void reload_donate_timer();

private:
	sl08::slot1<void, const int, IGame> reset_slot;
	void resetLoadingBar(const int total);

	sl08::slot2<void, const int, const char *, IGame> notify_slot;
	void notifyLoadingBar(const int progress, const char *what);

	sl08::slot1<void, const float, IGame> on_tick_slot;
	void onTick(const float dt);
	void tick(const float dt);

	sl08::slot2<bool, const SDL_keysym, const bool, IGame>  on_key_slot;
	bool onKey(const SDL_keysym sym, const bool pressed);

	sl08::slot3<void, const int, const int, const bool, IGame> on_joy_slot;
	void onJoyButton(const int, const int, const bool);

	sl08::slot4<bool, const int, const bool, const int, const int, IGame> on_mouse_slot;
	bool onMouse(const int button, const bool pressed, const int x, const int y);

	sl08::slot5<bool, const int, const int, const int, const int, const int, IGame> on_mouse_motion_slot;
	bool onMouseMotion(const int state, const int x, const int y, const int xrel, const int yrel);

	sl08::slot1<void, const std::string &, IGame> on_menu_slot;
	void onMenu(const std::string &name);

	sl08::slot0<void, IGame> on_map_slot;	
	void onMap();

	sl08::slot2<const std::string, const std::string &, const std::string &, IGame> on_console_slot;
	const std::string onConsole(const std::string &cmd, const std::string &param);

	sl08::slot1<void, const SDL_Event &, IGame> on_event_slot;
	void onEvent(const SDL_Event &event);

	void quit();
	
	void stopCutscene();

	bool _paused;

	MainMenu *_main_menu;
	
	bool _show_fps;
	const sdlx::Font * small_font;

	bool _autojoin;

	float _shake, _shake_max;
	int _shake_int;
	
	Hud *_hud;
	bool _show_stats;
	int _loading_bar_total, _loading_bar_now;
	
	Cutscene *_cutscene;
	Cheater *_cheater;
	
	const sdlx::Surface *_donate;
	float _donate_timer;
	
	Tooltip *_tip;
	Chat *_net_talk;
	bool server_running;
	int spawn_ai;

	std::vector<std::string> preload_map;
	RandomPool<size_t> preload_map_pool;
	void start_random_map();
	
	IGame(const IGame &);
	const IGame& operator=(const IGame &);
};

PUBLIC_SINGLETON(BTANKSAPI, Game, IGame);

#endif

