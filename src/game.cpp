/* Battle Tanks Game
 * Copyright (C) 2006-2007 Battle Tanks team
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

#include <stdlib.h>
#include <time.h>
#include "object.h"
#include "game.h"
#include "version.h"
#include "world.h"
#include "finder.h"
#include "resource_manager.h"
#include "game_monitor.h"

#include "tmx/map.h"

#include "mrt/lang.h"
#include "mrt/logger.h"
#include "mrt/exception.h"
#include "mrt/random.h"
#include "mrt/fs_node.h"
#include "mrt/directory.h"

#include "sdlx/system.h"
#include "sdlx/sdl_ex.h"
#include "sdlx/joystick.h"
#include "sdlx/color.h"
#include "sdlx/timer.h"

#include "net/server.h"
#include "net/client.h"
#include "net/protocol.h"
#include "net/connection.h"


#include "player_state.h"
#include "config.h"
#include "var.h"

#include "sound/mixer.h"
#include "player_slot.h"
#include "player_manager.h"
#include "hud.h"
#include "credits.h"
#include "cheater.h"
#include "console.h"
#include "special_zone.h"
#include "window.h"

#include "math/v3.h"
#include "menu/menu.h"
#include "menu/chat.h"
#include "menu/tooltip.h"

#include "i18n.h"
#include <math.h>
#include "special_owners.h"

IMPLEMENT_SINGLETON(Game, IGame);

IGame::IGame() : _main_menu(NULL),
 _autojoin(false), _shake(0), _show_stats(false), _credits(NULL), _cheater(NULL), _tip(NULL), _net_talk(NULL) {
 }
 
IGame::~IGame() {
	delete _net_talk;
}

void IGame::run() {
	Window->run();
}

void IGame::pause() {
	if (_main_menu->isActive())
		return;
	
	if (_paused) {
		_paused = false;
		return;
	}
	
	if (!PlayerManager->isServerActive())
		_paused = true;
}

void IGame::init(const int argc, char *argv[]) {
	srand(time(NULL));
	_donate = NULL;
	_donate_timer = 0;

	std::string path;
#ifdef PREFIX
	path = mrt::Directory::getAppDir("btanks") + "/";
#endif	
	Config->load(path + "bt.xml");
	
	{
		//setup some defaults
		
		int r;
		std::string s;
		Config->get("objects.alt-missiles-on-launcher.default-weapon", s, std::string());
		Config->get("objects.alt-missiles-on-launcher.default-weapon-type", s, std::string());
		Config->get("map.boomerang-missiles-item.respawn-interval", r, 15);
		Config->get("map.dirt-bullets-item.respawn-interval", r, 25);
		Config->get("map.dispersion-bullets-item.respawn-interval", r, 15);
		Config->get("map.megaheal.respawn-interval", r, 15);
		Config->get("map.mines-item.respawn-interval", r, 40);
		Config->get("map.nuke-missiles-item.respawn-interval", r, 20);
		Config->get("map.regular-mine.respawn-interval", r, 3600);
		Config->get("map.ricochet-bullets-item.respawn-interval", r, 20);
		Config->get("map.smoke-missiles-item.respawn-interval", r, 20);
		Config->get("map.stun-missiles-item.respawn-interval", r, 20);
		Config->get("map.machinegunner-item.respawn-interval", r, 20);
		Config->get("map.thrower-item.respawn-interval", r, 20);
		
		std::string s2;
		if (Config->has("player.control-method-1") && Config->has("player.control-method-2")) {
			Config->get("player.control-method-1", s, "keys-1");
			Config->get("player.control-method-2", s2, "keys-2");
			if (s == "keys" && s2 == "keys") {
				LOG_WARN(("bogus control methods found. fixing..."));
				Config->set("player.control-method-1", std::string("keys-1"));
				Config->set("player.control-method-2", std::string("keys-2"));
				Config->invalidateCachedValues();
			}
		}
	}
	{
		//place for upgrade.
		int revision;
		Config->get("engine.revision", revision, getRevision()); 
		if (revision < 1639) {
			int pfs;
			Config->get("engine.pathfinding-slice", pfs, 1);
			if (pfs > 1) {
				LOG_DEBUG(("upgrading engine.pathfinding-slice value. (reset it to 1)"));
				Config->set("engine.pathfinding-slice", 1);
			}
		}
		if (revision < 1852) {
			int rz;
			Config->get("hud.radar.zoom", rz, 2);
			if (rz > 2) {
				LOG_DEBUG(("decreasing hud.radar.zoom to 2"));
				Config->set("hud.radar.zoom", 2);
			}
		} 
		if (revision < 2614) {
			LOG_DEBUG(("upgrading map.thrower-item.respawn-interval and objects.shilka.special-fire-rate"));
			Config->set("map.thrower-item.respawn-interval", (int)45);
			Config->set("objects.shilka.special-fire-rate", (float)0.4);
		}
		if (revision < 2664) {
			Config->set("objects.zombie.reaction-time", (float)0.5);
		}
		if (revision < 2711) {
			Config->set("map.machinegunner-item.respawn-interval", 20);
			Config->set("map.thrower-item.respawn-interval", 20);		
		}
		if (revision < 3402) {
			Config->set("engine.sound.polling-interval", 1);
			Config->set("engine.sound.positioning-divisor", 40.0f);
			Config->remove("engine.sound.doppler-velocity");
		}
		if (revision < 3518) {	
			Config->remove("engine.sound.update-objects-interval");
			Config->set("engine.sound.buffers", 8);
		}
		if (revision < 3942) {	
			Config->set("engine.sound.file-buffer-size", 441000);
		}
		if (revision < 4009) {
			Config->set("engine.sound.maximum-sources", 16);
		}
		if (revision < 4317) {
			int cl;
			Config->get("multiplayer.compression-level", cl, 1);
			if (cl < 1) 
				Config->set("multiplayer.compression-level", 1);
		}
		if (revision < 4554) { //actually more revisions ago
			int fps_limit;
			Config->get("engine.fps-limit", fps_limit, 120);
			if (fps_limit >= 1000) 
				Config->set("engine.fps-limit", 120);
		}
		
		Config->set("engine.revision", getRevision());
	}

	GET_CONFIG_VALUE("engine.show-fps", bool, show_fps, true);
	GET_CONFIG_VALUE("engine.show-log-lines", bool, show_log_lines, false);
	
	_show_fps = show_fps;
	_show_log_lines = show_log_lines;

	GET_CONFIG_VALUE("engine.sound.disable-sound", bool, no_sound, false);
	GET_CONFIG_VALUE("engine.sound.disable-music", bool, no_music, false);
	
	std::string address, lang;
	
	for(int i = 1; i < argc; ++i) {
		if (strncmp(argv[i], "--connect=", 10) == 0) { address = argv[i] + 10; _autojoin = true; }
		else if (strncmp(argv[i], "--lang=", 7) == 0) { lang = argv[i] + 7; }
		else if (strcmp(argv[i], "--no-sound") == 0) { no_sound = true; no_music = true; }
		else if (strcmp(argv[i], "--help") == 0) { 
			printf(
					"\t--connect=ip/host\tconnect to given host as mp-client\n" 
					"\t--no-sound\t\tdisable sound.\n" 
				);
			Window->init(argc, argv);
			exit(0);
		}

	}
	if (lang.empty()) {
		if (Config->has("engine.language")) {
			Config->get("engine.language", lang, std::string());
		}

		if (lang.empty())
			lang = mrt::getLanguageCode();
	}
	
	IFinder::FindResult strings_files;
	Finder->findAll(strings_files, "strings.xml");
	for(size_t i = 0; i < strings_files.size(); ++i) 
		I18n->load(strings_files[i].second, lang);
	
	
	Window->init(argc, argv);

	IFinder::FindResult playlists;
	Finder->findAll(playlists, "playlist");
	if (playlists.empty())
		no_music = true;

	Mixer->init(no_sound, no_music);
	
	for(size_t i = 0; i < playlists.size(); ++i) 
		Mixer->loadPlaylist(playlists[i].second);
	
	Mixer->play();

	LOG_DEBUG(("probing for joysticks"));
	int jc = sdlx::Joystick::getCount();
	if (jc > 0) {
		LOG_DEBUG(("found %d joystick(s)", jc));
		//sdlx::Joystick::sendEvents(true);
		
		for(int i = 0; i < jc; ++i) {
			sdlx::Joystick j;
			j.open(i);
			LOG_DEBUG(("%d: %s axes: %d, buttons: %d, hats: %d, balls: %d", 
				i, sdlx::Joystick::getName(i).c_str(), 
				j.getNumAxes(), j.getNumButtons(), j.getNumHats(), j.getNumBalls()
				));
			
			j.close();
		}
	}
	Console->init();
	Console->on_command.connect(sigc::mem_fun(this, &IGame::onConsole));
	
	sdlx::Rect window_size = Window->getSize();
	if (_main_menu == NULL) {
		_main_menu = new MainMenu();
	}

	_paused = false;

	Window->getSurface().fill(0);
	Window->getSurface().flip();
	
	LOG_DEBUG(("initializing hud..."));
	_hud = new Hud(window_size.w, window_size.h);

	LOG_DEBUG(("installing callbacks..."));
	
	Window->key_signal.connect(sigc::mem_fun(this, &IGame::onKey));

	Window->mouse_signal.connect(sigc::mem_fun(this, &IGame::onMouse));
	Window->joy_button_signal.connect(sigc::mem_fun(this, &IGame::onJoyButton));
	Window->event_signal.connect(sigc::mem_fun(this, &IGame::onEvent));
	
	_main_menu->menu_signal.connect(sigc::mem_fun(this, &IGame::onMenu));
	
	Map->reset_progress.connect(sigc::mem_fun(this, &IGame::resetLoadingBar));
	Map->notify_progress.connect(sigc::mem_fun(this, &IGame::notifyLoadingBar));
	Map->load_map_signal.connect(sigc::mem_fun(this, &IGame::onMap));
	
	ResourceManager->reset_progress.connect(sigc::mem_fun(this, &IGame::resetLoadingBar));
	ResourceManager->notify_progress.connect(sigc::mem_fun(this, &IGame::notifyLoadingBar));

	Window->tick_signal.connect(sigc::mem_fun(this, &IGame::onTick));

	LOG_DEBUG(("initializing resource manager..."));
	
	std::vector<std::pair<std::string, std::string> > files;
	Finder->findAll(files, "resources.xml");
	
	ResourceManager->init(files);
	
	if (_show_fps) {
		LOG_DEBUG(("creating `digits' object..."));
		_fps = ResourceManager->createObject("damage-digits", "damage-digits");
		_fps->onSpawn();
		_fps->speed = 0;
	} else _fps = NULL;

	if (_show_log_lines) {
		LOG_DEBUG(("creating `digits' object..."));
		_log_lines = ResourceManager->createObject("damage-digits", "damage-digits");
		_log_lines->onSpawn();
		_log_lines->speed = 0;
	} else _log_lines = NULL;

	_main_menu->init(window_size.w, window_size.h);
	GET_CONFIG_VALUE("multiplayer.chat.lines-number", int, lines, 6);
	_net_talk = new Chat(lines);
	_net_talk->hide();

	if (_autojoin) {
		PlayerManager->startClient(address, 1);
		if (_main_menu)
			_main_menu->setActive(false);
	}
}

#include "controls/keyplayer.h"

bool IGame::onKey(const SDL_keysym key, const bool pressed) {
	if (_credits) {
		if (pressed)
			stopCredits();
		return true;
	}
	
	if (pressed && Map->loaded() && !_main_menu->isActive()) {
		if (_net_talk->hidden() && key.sym == SDLK_RETURN) {
			KeyPlayer::disable();
			_net_talk->hide(false);
		} else if (!_net_talk->hidden()) {
			_net_talk->onKey(key);
			if (_net_talk->changed()) {
				std::string message = _net_talk->get();
				
				_net_talk->reset();
				_net_talk->hide();
				KeyPlayer::enable();
				TRY {
					if (!message.empty())
						PlayerManager->say(message);
				} CATCH("say", throw);
			}
			return true;
		}
	}

	if (key.sym == SDLK_TAB) {
		_show_stats = pressed;
		return true;
	}

	if (!pressed)
		return false;

/*
-			case SDL_JOYBUTTONDOWN:
-				if (event.jbutton.button == 9) 
-					Game->pause();
-			break;
-			
*/
#	ifndef WIN32
	if (key.sym==SDLK_RETURN && key.mod & KMOD_CTRL) {
		TRY {
			Window->getSurface().toggleFullscreen();
		} CATCH("main loop", {});
		return true;
	}
#	endif
	if (key.sym == SDLK_PAUSE) {
		pause();
		return true;
	}
	if (key.sym==SDLK_s && key.mod & KMOD_SHIFT) {
		std::string path;
#ifdef PREFIX
		path = mrt::Directory::getAppDir("btanks") + "/";
#endif	
		static int n = 0; 
		std::string fname;
		do {
			fname = path + mrt::formatString("screenshot%02d.bmp", n++);
		} while(mrt::FSNode::exists(fname));
		LOG_DEBUG(("saving screenshot to %s", fname.c_str()));
		Window->getSurface().saveBMP(fname);
		return true;
	}
	if (key.sym==SDLK_m && key.mod & KMOD_SHIFT && Map->loaded()) {
		std::string path;
#ifdef PREFIX
		path = mrt::Directory::getAppDir("btanks") + "/";
#endif	
		const v2<int> msize = Map->getSize();
		LOG_DEBUG(("creating map screenshot %dx%d", msize.x, msize.y));

		sdlx::Surface screenshot;
		screenshot.createRGB(msize.x, msize.y, 32, SDL_SWSURFACE | SDL_SRCALPHA);
		screenshot.convertAlpha();
		screenshot.fillRect(screenshot.getSize(), screenshot.mapRGBA(0,0,0,255));

		sdlx::Rect viewport(0, 0, msize.x, msize.y);
		World->render(screenshot, viewport, viewport);
		screenshot.saveBMP(path + "map.bmp");
		return true;
	}

	if (key.sym == SDLK_m && !_main_menu->isActive()) {
		_hud->toggleMapMode();
		return true;
	}

	if (!PlayerManager->isClient() && key.sym==SDLK_F12 && PlayerManager->getSlotsCount() > 0) {
		PlayerSlot &slot = PlayerManager->getSlot(0);
		if (slot.frags > 0) 
			--slot.frags;

		Object *o = slot.getObject();
		if (o)
			o->emit("death", 0);
		return true;
	}

/*
*/

	if (key.sym == SDLK_ESCAPE) {
		if (!Map->loaded()) {
			if (_main_menu)
				_main_menu->setActive(true);
			return true;
		}
		
		LOG_DEBUG(("escape hit, paused: %s", _paused?"true":"false"));
		if (_main_menu)
			_main_menu->setActive(!_main_menu->isActive());
		
		if (PlayerManager->isServer() || PlayerManager->isClient()) {
			_paused = false;
		} else {
			if (_main_menu)
				_paused = _main_menu->isActive();
		}
		return true;
	}

	return false;
}

bool IGame::onMouse(const int button, const bool pressed, const int x, const int y) {
	if (pressed && _credits) {
		stopCredits();
		return true;
	}
	return false;
}

void IGame::onJoyButton(const int joy, const int id, const bool pressed) {
	if (pressed && id == 9) {
		pause();
	}
}

void IGame::onEvent(const SDL_Event &event) {
	if (event.type == SDL_QUIT)
		quit();
}

void IGame::onMenu(const std::string &name, const std::string &value) {
	if (name == "quit") {
		quit();
		//Window->stop();
	} else if (name == "credits" && !PlayerManager->isServerActive()) {
		LOG_DEBUG(("show credits."));
		_credits = new Credits;
	}
}

void IGame::stopCredits() {
	delete _credits;
	_credits = NULL;
	
	Window->resetTimer();
}


void IGame::quit() {
	_main_menu->setActive(false);

	GET_CONFIG_VALUE("engine.donate-screen-duration", float, dsd, 1.5f);
	if (dsd <= 0) {
		Window->stop();
		return;
	}
	_donate_timer = dsd;
	_donate = ResourceManager->loadSurface("donate.jpg");
}

void IGame::onTick(const float dt) {
	sdlx::Surface &window = Window->getSurface();
	int vx = 0, vy = 0;
	
	if (_donate_timer > 0 && _donate) {
		_donate_timer -= dt;
		if (_donate_timer <= 0) {
			Window->stop();
			return;
		}
		window.fill(0);
		sdlx::Rect window_size = Window->getSize();
		window.copyFrom(*_donate, (window_size.w - _donate->getWidth()) / 2, (window_size.h - _donate->getHeight()) / 2);
		goto flip;
	}

		if (Window->running() && !_paused) {
			GameMonitor->tick(dt);
			if (GameMonitor->gameOver()) {
				std::string type;
				Config->get("multiplayer.game-type", type, "deathmatch");
				if (type == "deathmatch")
					_show_stats = true;
			}
			PlayerManager->tick(SDL_GetTicks(), dt);
		}

		if (Map->loaded() && _credits == NULL && Window->running() && !_paused) {
			if (!PlayerManager->isClient())
				GameMonitor->checkItems(dt);
			PlayerManager->updatePlayers();
			
			Map->tick(dt);
			World->tick(dt);
		}

		Mixer->tick(dt);

		
		if (_main_menu)
			_main_menu->tick(dt);

		window.fill(0);

		if (!_credits && !Map->loaded())
			_hud->renderSplash(window);
		
		if (_credits) {
			_credits->render(dt, window);
			goto flip;
		}
	
		if (_shake > 0) {
			vy += _shake_int;
		}		

		PlayerManager->render(window, vx, vy);
		
		if (_shake > 0) {
			_shake_int = -_shake_int;
			_shake -= dt;
		}
		
		if (Map->loaded()) {
			_hud->render(window);

			const PlayerSlot *slot = PlayerManager->getMySlot();
			_hud->renderRadar(dt, window, GameMonitor->getSpecials(), 
				slot?sdlx::Rect((int)slot->map_pos.x, (int)slot->map_pos.y, slot->viewport.w, slot->viewport.h): sdlx::Rect());
			
			if (_main_menu && !_main_menu->isActive() && _show_stats) {
				_hud->renderStats(window);
			}

			if (!_net_talk->hidden()) {
				_net_talk->tick(dt);
				
			}
			_net_talk->render(window, 8, 32);
		}

		if (_main_menu)
			_main_menu->render(window);
		
		GameMonitor->render(window);		
		Console->render(window);
		
flip:
		float fr = Window->getFrameRate();
		if (_show_fps) {
			_fps->hp = (int)fr;
			_fps->render(window, window.getWidth() - (int)(_fps->size.x * 3), window.getHeight() - (int)_fps->size.y);
		}
		if (_show_log_lines) {
			_log_lines->hp = mrt::Logger->getLinesCounter();
			int size = (_log_lines->hp > 0)? (int)log10((double)_log_lines->hp) + 2:2;
			_log_lines->render(window, window.getWidth() - (int)(_log_lines->size.x * size), 20);
		}
		
		if (_paused) {
			static const sdlx::Font * font;
			if (font == NULL) 
				font = ResourceManager->loadFont("medium_dark", true);
			std::string pstr = I18n->get("messages", "game-paused");
			int w = font->render(NULL, 0, 0, pstr);
			font->render(window, (window.getWidth() - w) / 2, (window.getHeight() - font->getHeight()) / 2, pstr);
		}
}

void IGame::deinit() {
	clear();
	Mixer->deinit();
	
	delete _fps;
	_fps = NULL;

	delete _log_lines;
	_log_lines = NULL;
	
	delete _hud;
	_hud = NULL;
	
	if (_main_menu)
		_main_menu->deinit();

	delete _credits;
	_credits = NULL;
	
	delete _tip;
	_tip = NULL;

	ResourceManager->clear();

	TRY {
		Config->save();
	} CATCH("saving config", );

	//TTF_Quit();
	Window->deinit();
}



void IGame::clear() {
	LOG_DEBUG(("cleaning up main game object..."));
	Mixer->cancelAll();
	Mixer->reset();

	PlayerManager->clear();

	GameMonitor->clear();
	World->clear();

	_paused = false;
	_show_stats = false;
	Map->clear();
	
	delete _credits;
	_credits = NULL;
	
	delete _cheater;
	_cheater = NULL;

	if (_main_menu)
		_main_menu->setActive(true);

	if (_net_talk)
		_net_talk->clear();
}


void IGame::shake(const float duration, const int intensity) {
	_shake = duration;
	_shake_int = intensity;
}

void IGame::resetLoadingBar(const int total) {
	_loading_bar_now = 0;
	_loading_bar_total = total;
	
	std::deque<std::string> keys;
	I18n->enumerateKeys(keys, "tips");
	LOG_DEBUG(("%u tips found...", (unsigned)keys.size()));

	if (keys.empty())
		return;
	
	static std::deque<size_t> tips_available;
	if (tips_available.empty()) {
		for(size_t i = 0; i < keys.size(); ++i) 
			tips_available.push_back(i);
	}
	
	int i = mrt::random(tips_available.size());
	std::string tip = keys[tips_available[i]];
	{
		int n = i; std::deque<size_t>::iterator del = tips_available.begin();
		while(n--) 
			++del;
		tips_available.erase(del);
	}
	LOG_DEBUG(("showing tip: '%s', tips remaining: %u", tip.c_str(), (unsigned)tips_available.size()));

	delete _tip;
	_tip = new Tooltip(I18n->get("tips", tip), true, 320);
}

void IGame::notifyLoadingBar(const int progress) {
	GET_CONFIG_VALUE("hud.disable-loading-screen", bool, disable_bar, false);
	if (disable_bar)
		return;
	
	float old_progress = 1.0 * _loading_bar_now / _loading_bar_total;
	_loading_bar_now += progress;
	
	sdlx::Surface &window = Window->getSurface();
	const sdlx::Rect window_size = Window->getSize();
	if (_hud->renderLoadingBar(window, old_progress, 1.0 * _loading_bar_now / _loading_bar_total)) {
		if (_tip != NULL) {
			int w, h;
			_tip->getSize(w, h);
			_tip->render(window, (window_size.w - w) / 2, window_size.h - h * 5 / 4);
		}
		Window->flip();
		window.fill(0);
	}
}

const std::string IGame::onConsole(const std::string &cmd, const std::string &param) {
try {
	if (cmd == "quit") {
		//Window->stop();
		quit();
		return "thank you for playing battle tanks";
	} else if (cmd == "spawnplayer") {
		std::vector<std::string> par;
		mrt::split(par, param, " ", 3);
		if (par.size() < 3 || par[0].empty() || par[1].empty() || par[2].empty())
			return "usage: spawnPlayer object animation control-method";
		
		PlayerManager->spawnPlayer(par[0], par[1], par[2]);
		return "ok";
	} else if (cmd == "spawn") {
		std::vector<std::string> par;
		mrt::split(par, param, " ", 3);
			if (par.size() < 3 || par[0].empty() || par[1].empty() || par[2].empty())
				return "usage: spawn object animation position(10,20 /10,20 player5)";
			if (!Map->loaded())
				throw_ex(("map was not loaded"));
			v2<int> pos;
			bool tiled_pos = false;
			if (par[2][0] == '/') {
				tiled_pos = true;
				par[2] = par[2].substr(1);
			} 
			if (par[2].substr(0, 6) == "player") {
				int idx = par[2][6] - '0';
				Object *o = PlayerManager->getSlot(idx).getObject();
				if (o == NULL)
					throw_ex(("no object in slot %d", idx));
				o->getPosition(pos);
			} else pos.fromString(par[2]);
			if (tiled_pos) {
				v2<int> ts = Map->getTileSize();
				pos *= ts;
			}
			Object *o = ResourceManager->createObject(par[0], par[1]);
			o->addOwner(OWNER_MAP);
			World->addObject(o, pos.convert<float>());
			return "ok";
	} else if (cmd == "kill") {
		if (param.empty())
			return "usage: kill 0-n (slot number)";
		int idx = atoi(param.c_str());
		Object *o = PlayerManager->getSlot(idx).getObject();
		if (o == NULL)
			throw_ex(("no object in slot %d", idx));
		o->emit("death", NULL);
		return "ok";
	} else if (cmd == "setz") {		
		std::vector<std::string> p;
		mrt::split(p, param, " ");
		if (p.size() < 2)
			return "usage: setz <slot> <new z>";

		int idx = atoi(p[0].c_str());
		Object *o = PlayerManager->getSlot(idx).getObject();
		if (o == NULL)
			throw_ex(("no object in slot %d", idx));
		int z = atoi(p[1].c_str());
		o->setZ(z, true);
		return mrt::formatString("setting z %d for object %d", z, o->getID());
	} else if (cmd == "position") {
		if (param.empty())
			return "usage: position <slot>";
		int idx = atoi(param.c_str());
		Object *o = PlayerManager->getSlot(idx).getObject();
		if (o == NULL)
			throw_ex(("no object in slot %d", idx));

		v2<float> position;
		o->getCenterPosition(position);

		v2<int> tile_size = Map->getTileSize();
		v2<int> tiled = position.convert<int>() / tile_size;
		const std::string posstr = mrt::formatString("%g %g @%d,%d", position.x, position.y, tiled.x, tiled.y);
		LOG_NOTICE(("%s", posstr.c_str()));
		return posstr;
	} 

} catch(const std::exception &e) {
	return std::string("error: ") + e.what();
}
	return std::string();
}


void IGame::onMap() {
	_main_menu->setActive(false);

	delete _cheater;
	_cheater = NULL;
	if (!PlayerManager->isClient())
		_cheater = new Cheater;	
}

#include "sdlx/module.h"

void IGame::loadPlugins() {
	IFinder::FindResult path;
	Finder->findAll(path, "../" + sdlx::Module::mangle("bt_objects"));
	if (path.empty()) {
		std::vector<std::string> dirs;
		Finder->getPath(dirs);
		for(size_t i = 0; i < dirs.size(); ++i)
			dirs[i] += "/..";
		std::string dirs_str;
		mrt::join(dirs_str, dirs, " ");
		throw_ex(("engine could not find any 'bt_objects' shared libraries in the following directories: %s", dirs_str.c_str()));
	}
	
	for(IFinder::FindResult::const_iterator i = path.begin(); i != path.end(); ++i) {
		LOG_DEBUG(("loading plugin from %s", i->second.c_str()));
		sdlx::Module module;
		module.load(i->second);
		module.leak();
	}
}
