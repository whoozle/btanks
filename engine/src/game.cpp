/* Battle Tanks Game
 * Copyright (C) 2006-2009 Battle Tanks team
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

#include <stdlib.h>
#include <time.h>
#include "object.h"
#include "game.h"
#include "version.h"
#include "world.h"
#include "finder.h"
#include "resource_manager.h"
#include "game_monitor.h"
#include "rt_config.h"

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
#include "sdlx/font.h"

#include "net/server.h"
#include "net/client.h"
#include "net/message.h"
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
#include "menu/main_menu.h"
#include "menu/chat.h"
#include "menu/tooltip.h"
#include "nickname.h"

#include "i18n.h"
#define _USE_MATH_DEFINES
#include <math.h>
#include "special_owners.h"
#include "mrt/calendar.h"
#include "sdlx/cursor.h"
#include "logo.h"
#include "campaign.h"

IMPLEMENT_SINGLETON(Game, IGame);

IGame::IGame() : _main_menu(NULL),
 _autojoin(false), _shake(0), _shake_max(0), _show_stats(false), 
 _cutscene(NULL), _cheater(NULL), _tip(NULL), _net_talk(NULL), 
 spawn_ai(0) {

	std::string path;
	path = mrt::Directory::get_app_dir("Battle Tanks", "btanks") + "/";
	Config->load(path + "bt.xml");

#ifndef _WINDOWS
	std::string log;
	Config->get("engine.log", log, "log");
	if (!log.empty() && log != "stderr" && log != "<stderr>" ) {
		if (log[0] == '/') {
			mrt::Logger->assign(log);
		} else {
			mrt::Logger->assign(path + "/" + log);
		}
	}
#endif
	LOG_NOTICE(("starting up... version: %s", getVersion().c_str()));
}
 
IGame::~IGame() {
	delete _net_talk;
}

void IGame::stop() { 
	server_running = false; 
	Window->stop(); 
}

void IGame::run() {
	if (!RTConfig->server_mode) {
		Window->run();
	} else {
		server_running = true;
		LOG_DEBUG(("server is up and running!"));
		sdlx::Timer _timer;	
		
		int limit = 1000000 / 100;
		float dt = limit / 1000000.0f;
		while(server_running) {
			_timer.reset();
			if (!Map->loaded()) {
				start_random_map();
			}
			if (PlayerManager->is_server_active())
				tick(dt);
			else 
				PlayerManager->tick(dt);
			
			int t = _timer.microdelta();
			if (t < limit) {
				_timer.microsleep("server fps limit", limit - t);
			}
			dt = _timer.microdelta() / 1000000.0f;
		}
	}
}

void IGame::pause() {
	if (_main_menu == NULL || !_main_menu->hidden())
		return;
	
	if (_paused) {
		_paused = false;
		return;
	}
	
	if (!PlayerManager->is_server_active() && !PlayerManager->is_client())
		_paused = true;
}

void IGame::add_logo(sdlx::Surface * surface, float duration, Uint32 color, bool fade) {
	_logos.push_back(new Logo(surface, duration, color, fade));
}

void IGame::init(const int argc, char *argv[]) {
	_quit = false;
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
		if (revision < 5264) {
			std::set<std::string> keys;
			Config->enumerateKeys(keys, "objects.");
			for(std::set<std::string>::const_iterator i = keys.begin(); i != keys.end(); ++i) {
				const std::string &key = *i;
				int pos = (int)key.size() - 20;
				if (pos < 0) 
					continue;
				if (key.compare(pos, 20, "targeting-multiplier") == 0) {
					LOG_DEBUG(("removing invalid key: %s", key.c_str()));
					Config->remove(key);
				}
			}
		}
		if (revision < 5337) {
			if (Config->has("objects.ai-trooper.reaction-time")) {
				Config->remove("objects.ai-trooper.reaction-time");
				Config->remove("objects.trooper.reaction-time");
			}
		}
		if (revision < 5478) {
			Config->remove("engine.preload-all-resources");
		}
		if (revision < 5587) {
			float mp;
			Config->get("objects.mutagen-explosion.mutation-probability", mp, 0.5f);
			if (mp < 0.5f) 
				Config->set("objects.mutagen-explosion.mutation-probability", 0.5f);
		}
		if (revision <= 5646) {
			Config->remove("objects.car.reaction-time");
			Config->remove("objects.buggy.reaction-time");
			Config->remove("objects.civilian.reaction-time");
			Config->remove("objects.combine.reaction-time");
			Config->remove("objects.tractor.reaction-time");
		}
		if (revision < 5700) {
			Config->remove("engine.pathfinding-throttling");
		}
		if (revision < 5829 && Config->has("multiplayer.port")) {
			Config->set("multiplayer.port", 27255);
		}
		if (revision < 6205) { //actually more revisions ago
			int fps_limit;
			Config->get("engine.fps-limit", fps_limit, 100);
			if (fps_limit > 100 || fps_limit == 50) 
				Config->set("engine.fps-limit", 100);
		}
		if (revision < 6571) {
			Config->set("engine.sound.sample-rate", 22050);
		}
		if (revision < 6831) {
			Config->remove("objects.kamikaze.reaction-time");
		}
		if (revision < 6850) {
			Config->remove("engine.path");
		}
		if (revision < 6865) {
			int pfs;
			Config->get("engine.pathfinding-slice", pfs, 2);
			if (pfs < 2) {
				LOG_DEBUG(("upgrading engine.pathfinding-slice value. (reset it to 2)"));
				Config->set("engine.pathfinding-slice", 2);
			}
		}
		if (revision < 7292) {
			Config->remove("menu.state");
			Config->remove("menu.default-mp-map");
		}
		if (revision < 7391) {
			int cl;
			Config->get("multiplayer.compression-level", cl, 3);
			if (cl < 3) 
				Config->set("multiplayer.compression-level", 3);
		}
		if (revision < 7540) {
			int sid;
			Config->get("multiplayer.sync-interval-divisor", sid, 5);
			if (sid > 5) 
				Config->set("multiplayer.sync-interval-divisor", 5);
		}
		if (revision < 7574) {
			int ds;
			Config->get("multiplayer.deltas-samples", ds, 15);
			if (ds < 15)
				Config->set("multiplayer.deltas-samples", 15);
		}
		if (revision < 7888) {
			Config->remove("engine.sound.positioning-divisor");
		}
		
		if (revision < 8041) {
			std::set<std::string> keys;
			Config->enumerateKeys(keys, "player.");
			for(std::set<std::string>::const_iterator i = keys.begin(); i != keys.end(); ++i) {
				std::string id = "profile.0." + i->substr(7);
				LOG_DEBUG(("renaming %s to %s", i->c_str(), id.c_str()));
				Config->rename(*i, id);
			}
		}
		if (revision < 8061) {
			float fx_volume;
			Config->get("engine.sound.volume.fx", fx_volume, 0.66f);
			if (fx_volume >= 1.0f)
				Config->set("engine.sound.volume.fx", 0.66f);
		}
		
		Config->set("engine.revision", getRevision());
	}

	Config->get("multiplayer.port", RTConfig->port, 27255);
	Config->get("engine.show-fps", _show_fps, false);
	
	GET_CONFIG_VALUE("engine.sound.disable-sound", bool, no_sound, false);
	GET_CONFIG_VALUE("engine.sound.disable-music", bool, no_music, false);
	
	std::string lang, bind;
	bool xmas = mrt::xmas();
	for(int i = 1; i < argc; ++i) {
		if (strncmp(argv[i], "--connect=", 10) == 0) { _address = argv[i] + 10; _autojoin = true; }
		else if (strncmp(argv[i], "--bind=", 7) == 0) { bind = argv[i] + 7; }
		else if (strncmp(argv[i], "--lang=", 7) == 0) { lang = argv[i] + 7; }
		else if (strncmp(argv[i], "--map=", 6) == 0) { mrt::split(preload_map, argv[i] + 6, ","); preload_map_pool.init(0, preload_map.size()); }
		else if (strcmp(argv[i], "--no-sound") == 0) { no_sound = true; no_music = true; }
		else if (strcmp(argv[i], "--xmas") == 0) { xmas = true; }
		else if (strcmp(argv[i], "--no-xmas") == 0) { xmas = false; }
		else if (strcmp(argv[i], "--sound") == 0) { no_sound = false; no_music = false; }
		else if (strcmp(argv[i], "--server") == 0) { RTConfig->server_mode = true; }
		else if (strncmp(argv[i], "--ai=", 5) == 0) { spawn_ai = atoi(argv[i] + 5); }
		else if (strncmp(argv[i], "--game-type=", 12) == 0) { RTConfig->game_type = IRTConfig::parse_game_type(argv[i] + 12); RTConfig->teams = 2; }
		else if (strncmp(argv[i], "--time-limit=", 13) == 0) { RTConfig->time_limit = (float)atof(argv[i] + 13); }
		else if (strncmp(argv[i], "--port=", 7) == 0) { RTConfig->port = atoi(argv[i] + 7); if (RTConfig->port <= 0) throw_ex(("invalid port specified: %d", RTConfig->port)); }
		else if (strncmp(argv[i], "--log=", 6) == 0) { mrt::Logger->assign(argv[i] + 6); }
		else if (strcmp(argv[i], "--help") == 0) { 
			Window->init(argc, argv);
			printf( 
					"\t--connect=ip/host\tconnect to given host as mp-client\n" 
					"\t--no-sound\t\tdisable sound.\n" 
					"\t--lang\t\t\tswitch language (2-letter iso code: en, ru, de...)\n"
					"\t--xmas\t\t\tswitch xmas mode on\n"
					"\t--no-xmas\t\tswitch xmas mode off\n"
					"\n\tWARNING: options below are for advanced users and will not affect gameplay\n"
					"\t--server\t\tswitch to server mode [no gui]\n"
					"\t--port\t\t\tuse specified port, default: 27255\n"
					"\t--map\t\t\tcomma separated map list\n"
					"\t--ai\t\t\tadd prespawned ai players\n"
					"\t--game-type\t\tforce game type for server mode. [deathmatch, team-deathmatch, ctf]\n"
					"\t--time-limit\t\tsets time limit (in seconds)\n"
				);
			exit(0);
		}
	}
	if (RTConfig->server_mode) {
		no_sound = no_music = true;
	}
	if (!bind.empty()) {
		Var v("string");
		v.s = bind;
		Config->setOverride("multiplayer.bind-address", v);
	}
	
	if (lang.empty()) {
		if (Config->has("engine.language")) {
			Config->get("engine.language", lang, std::string());
		}

		if (lang.empty())
			lang = mrt::get_lang_code();
	}
	
	
	if (xmas) {
		LOG_DEBUG(("xmas mode!"));
		Finder->addPatchSuffix("_xmas");
	}
	
	I18n->load(lang);
	
	if (!RTConfig->server_mode) {
		Window->init(argc, argv);
	} else {
		Window->init_dummy();
	}

	IFinder::FindResult playlists;
	Finder->findAll(playlists, "playlist");
	if (playlists.empty())
		no_music = true;

	Mixer->init(no_sound, no_music);
	
	for(size_t i = 0; i < playlists.size(); ++i) 
		Mixer->loadPlaylist(playlists[i].second);
	
	Mixer->play();

	if (!RTConfig->server_mode) {
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
					j.get_axis_num(), j.get_buttons_num(), j.get_hats_num(), j.get_balls_num()
				));
			
				j.close();
			}
		}
		Console->init();
		on_console_slot.assign(this, &IGame::onConsole, Console->on_command);

		LOG_DEBUG(("installing basic callbacks..."));
		on_key_slot.assign(this, &IGame::onKey, Window->key_signal);
		on_mouse_slot.assign(this, &IGame::onMouse, Window->mouse_signal);
		on_mouse_motion_slot.assign(this, &IGame::onMouseMotion, Window->mouse_motion_signal);
		on_joy_slot.assign(this, &IGame::onJoyButton, Window->joy_button_signal);
		on_event_slot.assign(this, &IGame::onEvent, Window->event_signal);
	}

	_paused = false;

	if (!RTConfig->server_mode) {
		Window->get_surface().fill(0);
		Window->get_surface().flip();
	
		LOG_DEBUG(("initializing hud..."));
		sdlx::Rect window_size = Window->get_size();
		_hud = new Hud(window_size.w, window_size.h);
	} else {  
		_hud = NULL;
	}

	LOG_DEBUG(("installing callbacks..."));
	
	if (!RTConfig->server_mode) {
		on_map_slot.assign(this, &IGame::onMap, Map->load_map_signal);

		on_logo_tick_slot.assign(this, &IGame::logo_tick, Window->tick_signal);
		on_tick_slot.assign(this, &IGame::onTick, Window->tick_signal);
	}

	reset_slot.assign(this, &IGame::resetLoadingBar, Map->reset_progress);
	notify_slot.assign(this, &IGame::notifyLoadingBar, Map->notify_progress);

	reset_slot.assign(this, &IGame::resetLoadingBar, ResourceManager->reset_progress);
	notify_slot.assign(this, &IGame::notifyLoadingBar, ResourceManager->notify_progress);

	_need_postinit = true;

	parse_logos();
}

void IGame::resource_init() {
	LOG_DEBUG(("initializing resource manager..."));
	_need_postinit = false;
	
	std::vector<std::pair<std::string, std::string> > files;
	Finder->findAll(files, "resources.xml");
	
	ResourceManager->init(files);
	
	if (_main_menu == NULL && !RTConfig->server_mode) {
		LOG_DEBUG(("initializing main menu..."));
		sdlx::Rect size = Window->get_size();
		delete _main_menu;
		_main_menu = new MainMenu(size.w, size.h);
		on_menu_slot.assign(this, &IGame::onMenu, _main_menu->menu_signal);
	}
	
	if (!RTConfig->server_mode) {
		if (_show_fps) {
			small_font = ResourceManager->loadFont("small", true);
		}
	
		_net_talk = new Chat();
		_net_talk->hide();

		if (_autojoin && !RTConfig->disable_network) {
			mrt::Socket::addr addr;
			addr.parse(_address);
			PlayerManager->start_client(addr, 1);
			if (_main_menu)
				_main_menu->hide();
		}
	} else {
		_net_talk = NULL;
	}

	start_random_map();
}

bool IGame::logo_tick(const float dt) {
	if (_quit) {
		Window->stop();
		return true;
	}

	if (_cutscene == NULL) {
		if (_logos.empty())
			return false;

		_cutscene = _logos.front();
		_logos.pop_front();
	} else {
		_cutscene->render(dt, Window->get_surface());
		if (_cutscene->finished())
			stop_cutscene();
	}
	return true;
}

void IGame::parse_logos() {
	LOG_DEBUG(("searching for prestart stuff: logos..."));
	IFinder::FindResult files;

	Finder->findAll(files, "campaign.xml");
	if (files.empty()) {
		return;
	}

	LOG_DEBUG(("found %u campaign(s)", (unsigned)files.size()));
	std::vector<std::string> titles;

	for(size_t i = 0; i < files.size(); ++i) {
		LOG_DEBUG(("campaign[%u](preparse): %s %s", (unsigned)i, files[i].first.c_str(), files[i].second.c_str()));
		Campaign c;
		c.init(files[i].first, files[i].second, true);
		RTConfig->disable_donations |= c.disable_donations;
		RTConfig->disable_network |= c.disable_network;
	}
}

void IGame::start_random_map() {
	if (preload_map.empty()) 
		return;
	
	std::string map = preload_map[preload_map_pool.get()];
	mrt::trim(map);
	
	GameMonitor->startGame(NULL, map);
	for(int i = 0; i < spawn_ai; ++i) {
		const char *c_vehicle[] = {"tank", "shilka", "launcher", };
		std::string vehicle = c_vehicle[mrt::random(3)], animation;
		const int slot_id = PlayerManager->find_empty_slot();
		PlayerSlot &slot = PlayerManager->get_slot(slot_id);
		
		slot.getDefaultVehicle(vehicle, animation);
		slot.name = Nickname::generate();
		LOG_DEBUG(("player%d: %s:%s, name: %s", slot_id, vehicle.c_str(), animation.c_str(), slot.name.c_str()));
		slot.spawn_player(slot_id, vehicle, animation);
	}
}

#include "controls/keyplayer.h"

bool IGame::onKey(const SDL_keysym key, const bool pressed) {
	if (_cutscene) {
		if (pressed)
			stop_cutscene();
		return true;
	}
	
	if (pressed && Map->loaded() && _main_menu->hidden()) {
		if (_net_talk->hidden() && key.sym == SDLK_RETURN) {
			_net_talk->hide(false);
		} else if (!_net_talk->hidden()) {
			_net_talk->onKey(key);
			if (_net_talk->changed()) {
				std::string message = _net_talk->get();
				
				_net_talk->reset();
				_net_talk->hide();
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
#	ifndef _WINDOWS
	if (key.sym==SDLK_RETURN && key.mod & KMOD_CTRL) {
		TRY {
			Window->get_surface().toggle_fullscreen();
		} CATCH("main loop", {});
		return true;
	}
#	endif
	if (key.sym == SDLK_PAUSE) {
		pause();
		return true;
	}
	if (key.sym==SDLK_s && key.mod & KMOD_SHIFT) {
		std::string path = mrt::Directory::get_app_dir("Battle Tanks", "btanks") + "/";
		std::string name = Map->getName();
		path += name.empty()?"screenshot":name;

		int n = 1;
		std::string fname;
		mrt::Directory dir;
		do {
			fname = path + mrt::format_string("%02d.bmp", n++);
		} while(dir.exists(fname));
		LOG_DEBUG(("saving screenshot to %s", fname.c_str()));
		TRY {
			Window->get_surface().save_bmp(fname);
		} CATCH("saving screenshot", {});
		return true;
	}
	if (key.sym==SDLK_m && key.mod & KMOD_SHIFT && Map->loaded()) {
		std::string path = mrt::Directory::get_app_dir("Battle Tanks", "btanks") + "/";
		std::string name = Map->getName();
		path += name.empty()?"map":name;
		path += ".bmp";
		
		const v2<int> msize = Map->get_size();
		LOG_DEBUG(("creating map screenshot %dx%d", msize.x, msize.y));

		sdlx::Surface screenshot;
		screenshot.create_rgb(msize.x, msize.y, 32, SDL_SWSURFACE | SDL_SRCALPHA);
		screenshot.display_format_alpha();
		screenshot.fill_rect(screenshot.get_size(), screenshot.map_rgba(0,0,0,255));

		sdlx::Rect viewport(0, 0, msize.x, msize.y);
		World->render(screenshot, viewport, viewport);
		TRY {
			screenshot.save_bmp(path);
		} CATCH("saving screenshot", {});
		return true;
	}

	if (key.sym == SDLK_m && _main_menu->hidden()) {
		_hud->toggleMapMode();
		return true;
	}

	if (!PlayerManager->is_client() && key.sym==SDLK_F12 && PlayerManager->get_slots_count() > 0) {
		TRY {
			PlayerSlot *slot = PlayerManager->get_my_slot();
			if (slot == NULL)
				return true;
		
			Object *o = slot->getObject();
			if (o)
				o->emit("death", o);
		} CATCH("f12-suicide", {});
		return true;
	}

/*
*/
	if (_main_menu && _main_menu->onKey(key))
		return true;

	if (key.sym == SDLK_ESCAPE) {
		if (_main_menu && _main_menu->hidden()) {
			_main_menu->hide(false);
			return true;
		}
	}

	return false;
}

bool IGame::onMouse(const int button, const bool pressed, const int x, const int y) {
	if (_cutscene) {
		if (!pressed)
			stop_cutscene();
		return true;
	}
	return _main_menu && _main_menu->onMouse(button, pressed, x, y);
}

bool IGame::onMouseMotion(const int state, const int x, const int y, const int xrel, const int yrel) {
	if (_cutscene)
		return true;
	return _main_menu && _main_menu->onMouseMotion(state, x, y, xrel, yrel);
}


void IGame::onJoyButton(const int joy, const int id, const bool pressed) {
	if (pressed && id == 9) {
		pause();
	}
}

void IGame::onEvent(const SDL_Event &event) {
	if (_main_menu)
		_main_menu->onEvent(event);

	if (event.type == SDL_QUIT)
		quit();
		
	if (event.type == SDL_ACTIVEEVENT) {
		const SDL_ActiveEvent & active = event.active;
		if (active.state == SDL_APPMOUSEFOCUS)
			return;
		
		LOG_DEBUG(("active event: %d, %d", active.state, active.gain));
		if (active.gain == 0 && !_paused)
			pause();
	}
	
	if (_paused && (event.type == SDL_KEYDOWN || event.type == SDL_MOUSEBUTTONDOWN))
		pause();
}

void IGame::onMenu(const std::string &name) {
	if (name == "quit") {
		quit();
		//Window->stop();
	} else if (name == "credits" && !PlayerManager->is_server_active()) {
		LOG_DEBUG(("show credits."));
		_cutscene = new Credits;
	}
}

void IGame::stop_cutscene() {
	delete _cutscene;
	_cutscene = NULL;
	
	Window->resetTimer();
}


void IGame::quit() {
	if (_main_menu)
		_main_menu->hide();
	
	_quit = true;

	if (RTConfig->disable_donations)
		return;

	float duration;
	Config->get("engine.donate-screen-duration", duration, 1.5f);
	if (duration < 0.1f)
		return;

	sdlx::Surface *s = NULL;
	try {
		mrt::Chunk data;
		std::string tname = "tiles/donate.jpg";
		Finder->load(data, tname);

		s = new sdlx::Surface;
		s->load_image(data);
		s->display_format();


		add_logo(s, duration, 0, false);
	} CATCH("showing donate screen", delete s);

}

bool IGame::tick(const float dt) {
	GameMonitor->tick(dt);
	if (Map->loaded()) {
		GameMonitor->checkItems(dt);
			
		Map->tick(dt);
		World->tick(dt);
		World->purge(dt);

		PlayerManager->update_players(dt);
		PlayerManager->tick(dt);
	}
	return true;
}

bool IGame::onTick(const float dt) {
	if (_quit) {
		Window->stop();
		return true;
	}
	
	if (_need_postinit)
		resource_init();

	sdlx::Surface &window = Window->get_surface();
	int vx = 0, vy = 0;

	if (Window->running() && !_paused) {
		GameMonitor->tick(dt);
		if (GameMonitor->game_over()) {
			_show_stats = true;
		}
	}

	if (Map->loaded() && Window->running() && !_paused) {
		if (!PlayerManager->is_client())
			GameMonitor->checkItems(dt);
		
		Map->tick(dt);
		World->tick(dt);
		PlayerManager->update_players(dt);
		World->purge(dt);
	}

	if (Window->running() && !_paused) {
		PlayerManager->tick(dt); //avoid any dead objects in serialization
	}

	Mixer->tick(dt);

	
	if (_main_menu) {
		_main_menu->tick(dt);
		bool cursor = sdlx::Cursor::enabled();
		bool menu = !_main_menu->hidden();
		if (!menu && cursor) {
			sdlx::Cursor::Disable();
		} else if (menu && !cursor) {
			sdlx::Cursor::Enable();
		}
	}

	window.fill(window.map_rgb(0x10, 0x10, 0x10));

	if (!Map->loaded())
		_hud->renderSplash(window);
		
	if (_shake > 0) {
		vy += (int)floor(_shake_int * 5 * sin((1 - _shake / _shake_max) * M_PI * 2 * 6) * (_shake / _shake_max));
		//vy += _shake_int;
	}		

	PlayerManager->render(window, vx, vy);
		
	if (_shake > 0) {
		//_shake_int = -_shake_int;
		_shake -= dt;
	}
		
	if (Map->loaded()) {
		_hud->render(window);

		const PlayerSlot *slot = PlayerManager->get_my_slot();
		_hud->renderRadar(dt, window, GameMonitor->getSpecials(), GameMonitor->getFlags(), 
			slot?sdlx::Rect((int)slot->map_pos.x, (int)slot->map_pos.y, slot->viewport.w, slot->viewport.h): sdlx::Rect());
			
		if (_main_menu && _main_menu->hidden() && _show_stats) {
			_hud->renderStats(window);
		}

		if (_net_talk != NULL) {
			_net_talk->tick(dt);
		}
		_net_talk->render(window, 8, 32);
	}

	if (_main_menu) {
		_main_menu->render(window, 0, 0);
	}
		
	GameMonitor->render(window);
	Console->render(window);

	if (_show_fps && small_font) {
		float fr = Window->getFrameRate();
		std::string fps = mrt::format_string("%d", (int)fr);
		int w = small_font->render(NULL, 0, 0, fps);
		small_font->render(window, window.get_width() - w, window.get_height() - small_font->get_height(), fps);
	}

	if (_paused) {
		static const sdlx::Font * font;
		if (font == NULL) 
			font = ResourceManager->loadFont("medium_dark", true);
		std::string pstr = I18n->get("messages", "game-paused");
		int w = font->render(NULL, 0, 0, pstr);
		font->render(window, (window.get_width() - w) / 2, (window.get_height() - font->get_height()) / 2, pstr);
	}

	return true;
}

void IGame::deinit() {
	clear();
	Mixer->deinit();
	
	delete _hud;
	_hud = NULL;

	delete _cutscene;
	_cutscene = NULL;
	
	delete _tip;
	_tip = NULL;
	
	delete _main_menu;
	_main_menu = NULL;

	ResourceManager->clear();

	TRY {
		Config->save();
	} CATCH("saving config", {});

	//TTF_Quit();
	Window->deinit();
}



void IGame::clear() {
	LOG_DEBUG(("cleaning up main game object..."));
	Mixer->cancel_all();
	Mixer->reset();

	PlayerManager->clear();

	GameMonitor->clear();
	World->clear();

	_paused = false;
	_show_stats = false;
	Map->clear();
	
	delete _cutscene;
	_cutscene = NULL;
	
	delete _cheater;
	_cheater = NULL;

	if (_main_menu)
		_main_menu->hide(false);

	if (_net_talk)
		_net_talk->clear();
}


void IGame::shake(const float duration, const int intensity) {
	_shake = duration;
	_shake_max = duration;
	_shake_int = intensity;
}

void IGame::resetLoadingBar(const int total) {
	_loading_bar_now = 0;
	_loading_bar_total = total;

	if (RTConfig->server_mode)
		return;
	
	std::deque<std::string> keys;
	I18n->enumerateKeys(keys, "tips/");
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
	_tip = new Tooltip("tips", tip, true, 320);
}

void IGame::notifyLoadingBar(const int progress, const char *what) {
	GET_CONFIG_VALUE("hud.disable-loading-screen", bool, disable_bar, false);
	if (disable_bar)
		return;

	if (RTConfig->server_mode) {
		int p0 = 10 * _loading_bar_now / _loading_bar_total;
		_loading_bar_now += progress;
		int p1 = 10 * _loading_bar_now / _loading_bar_total;
		if (p0 != p1) {
			LOG_DEBUG(("%d0%%", p1));
		}
		return;
	}
	
	float old_progress = 1.0f * _loading_bar_now / _loading_bar_total;
	_loading_bar_now += progress;
	
	sdlx::Surface &window = Window->get_surface();
	const sdlx::Rect window_size = Window->get_size();
	if (_hud->renderLoadingBar(window, old_progress, 1.0f * _loading_bar_now / _loading_bar_total, what)) {
		if (_tip != NULL) {
			int w, h;
			_tip->get_size(w, h);
			_tip->render(window, (window_size.w - w) / 2, window_size.h - h * 5 / 4);
		}
		Window->flip();
		window.fill(window.map_rgb(0x10, 0x10, 0x10));
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
			return "usage: spawn_player object animation control-method";
		
		PlayerManager->spawn_player(par[0], par[1], par[2]);
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
			if (par[2].compare(0, 6, "player") == 0) {
				int idx = par[2][6] - '0';
				Object *o = PlayerManager->get_slot(idx).getObject();
				if (o == NULL)
					throw_ex(("no object in slot %d", idx));
				o->get_position(pos);
			} else pos.fromString(par[2]);
			if (tiled_pos) {
				v2<int> ts = Map->getTileSize();
				pos *= ts;
			}
			Object *o = ResourceManager->createObject(par[0], par[1]);
			o->add_owner(OWNER_MAP);
			World->addObject(o, pos.convert<float>());
			return "ok";
	} else if (cmd == "kill") {
		if (param.empty())
			return "usage: kill 0-n (slot number)";
		int idx = atoi(param.c_str());
		Object *o = PlayerManager->get_slot(idx).getObject();
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
		Object *o = PlayerManager->get_slot(idx).getObject();
		if (o == NULL)
			throw_ex(("no object in slot %d", idx));
		int z = atoi(p[1].c_str());
		o->set_z(z, true);
		return mrt::format_string("setting z %d for object %d", z, o->get_id());
	} else if (cmd == "position") {
		if (param.empty())
			return "usage: position <slot>";
		int idx = atoi(param.c_str());
		Object *o = PlayerManager->get_slot(idx).getObject();
		if (o == NULL)
			throw_ex(("no object in slot %d", idx));

		v2<float> position;
		o->get_center_position(position);

		v2<int> tile_size = Map->getTileSize();
		v2<int> tiled = position.convert<int>() / tile_size;
		const std::string posstr = mrt::format_string("%g %g @%d,%d", position.x, position.y, tiled.x, tiled.y);
		LOG_NOTICE(("%s", posstr.c_str()));
		return posstr;
	} 

} catch(const std::exception &e) {
	return std::string("error: ") + e.what();
}
	return std::string();
}


void IGame::onMap() {
	if (_main_menu != NULL) {
		LOG_DEBUG(("hiding main menu"));
		_main_menu->hide();
	}

	delete _cheater;
	_cheater = NULL;
	if (!PlayerManager->is_client())
		_cheater = new Cheater;	
}

#include "sdlx/module.h"

void IGame::loadPlugins() {
	LOG_DEBUG(("loading plugins..."));
	IFinder::FindResult path;
	std::string plugin = std::string("../") + sdlx::Module::mangle("bt_objects");
	//LOG_DEBUG(("checking %s", plugin.c_str()));
	Finder->findAll(path, plugin);
/*
#ifdef __APPLE__
	{
		char buf[1024];
		getcwd(buf, sizeof(buf));
		LOG_DEBUG(("current directory: %s", buf));
		mrt::FSNode dir;
		std::string local_plugin = sdlx::Module::mangle("bt_objects");
		LOG_DEBUG(("checking for plugin %s", local_plugin.c_str()));
		if (dir.exists(std::string("./") + local_plugin))
			path.push_back(IFinder::FindResult::value_type(std::string(), local_plugin));
	}
#endif
*/
#ifdef PLUGINS_DIR
	{
		mrt::FSNode dir;
		std::string local_plugin = std::string(PLUGINS_DIR "/") + sdlx::Module::mangle("bt_objects");
		if (dir.exists(local_plugin))
			path.push_back(IFinder::FindResult::value_type(PLUGINS_DIR "/", local_plugin));
	}
#endif

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
		if (i->second.find('/') != std::string::npos)
			module.load(i->second);
		else 
			module.load("./" + i->second); //return same handle for the dll :(( private/bt_objects.dll, bt_objects.dll		
		module.leak();
	}
}
