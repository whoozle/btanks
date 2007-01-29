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
#include "resource_manager.h"

#include "tmx/map.h"

#include "mrt/logger.h"
#include "mrt/exception.h"
#include "mrt/random.h"

#include "sdlx/system.h"
#include "sdlx/sdl_ex.h"
#include "sdlx/joystick.h"
#include "sdlx/ttf.h"
#include "sdlx/color.h"

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

//#define SHOW_PERFSTATS

IMPLEMENT_SINGLETON(Game, IGame)

IGame::IGame() : 
_check_items(0.5, true),  _autojoin(false), _shake(0), _credits(NULL), _cheater(NULL) {}
IGame::~IGame() {}

void IGame::init(const int argc, char *argv[]) {
	srand(time(NULL));
	
	Config->load("bt.xml");
	
	{
		//setup some defaults
		
		int r;
		Config->get("map.boomerang-missiles-item.respawn-interval", r, 15);
		Config->get("map.dirt-bullets-item.respawn-interval", r, 25);
		Config->get("map.dispersion-bullets-item.respawn-interval", r, 15);
		Config->get("map.machinegunner-item.respawn-interval", r, 45);
		Config->get("map.megaheal.respawn-interval", r, 15);
		Config->get("map.mines-item.respawn-interval", r, 40);
		Config->get("map.nuke-missiles-item.respawn-interval", r, 20);
		Config->get("map.regular-mine.respawn-interval", r, 3600);
		Config->get("map.ricochet-bullets-item.respawn-interval", r, 20);
		Config->get("map.smoke-missiles-item.respawn-interval", r, 20);
		Config->get("map.stun-missiles-item.respawn-interval", r, 20);
	}
	{
		//place for upgrade.
		int revision;
		Config->get("engine.revision", revision, 1638); //this key first time appears in 1638 
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
		Config->set("engine.revision", getRevision());
	}

	GET_CONFIG_VALUE("engine.show-fps", bool, show_fps, true);
	GET_CONFIG_VALUE("engine.data-directory", std::string, data_dir, "data");
	
	_show_fps = show_fps;


	GET_CONFIG_VALUE("engine.sound.disable-sound", bool, no_sound, false);
	GET_CONFIG_VALUE("engine.sound.disable-music", bool, no_music, false);
	
	std::string address;
	
	for(int i = 1; i < argc; ++i) {
		if (strncmp(argv[i], "--connect=", 10) == 0) { address = argv[i] + 10; _autojoin = true; }
		else if (strcmp(argv[i], "--no-sound") == 0) { no_sound = true; no_music = true; }
		else if (strcmp(argv[i], "--help") == 0) { 
			printf(
					"\t--connect=ip/host\tconnect to given host as mp-client\n" 
					"\t--no-sound\t\tdisable sound.\n" 
				);
			Window::init(argc, argv);
			exit(0);
		}

	}
	
	
	Window::init(argc, argv);
	Mixer->init(no_sound, no_music);
	
	Mixer->loadPlaylist(data_dir + "/playlist");
	Mixer->play();

	LOG_DEBUG(("probing for joysticks"));
	int jc = sdlx::Joystick::getCount();
	if (jc > 0) {
		LOG_DEBUG(("found %d joystick(s)", jc));
		//sdlx::Joystick::sendEvents(true);
		
		for(int i = 0; i < jc; ++i) {
			LOG_DEBUG(("%d: %s", i, sdlx::Joystick::getName(i).c_str()));
/*			sdlx::Joystick j;
			j.open(i);
			
			j.close();
*/
		}
	}
	

	LOG_DEBUG(("initializing menus..."));		
	_main_menu.init(_window.getWidth(), _window.getHeight());	

	_paused = false;
	_running = true;
	_map_loaded = false;

	_window.fillRect(_window.getSize(), 0);
	_window.flip();
	
	LOG_DEBUG(("initializing hud..."));
	_hud = new Hud(_window.getWidth(), _window.getHeight());
	
	Console->on_command.connect(sigc::mem_fun(this, &IGame::onConsole));

	LOG_DEBUG(("installing callbacks..."));
	key_signal.connect(sigc::mem_fun(this, &IGame::onKey));
	_main_menu.menu_signal.connect(sigc::mem_fun(this, &IGame::onMenu));
	
	Map->reset_progress.connect(sigc::mem_fun(this, &IGame::resetLoadingBar));
	Map->notify_progress.connect(sigc::mem_fun(this, &IGame::notifyLoadingBar));
	ResourceManager->reset_progress.connect(sigc::mem_fun(this, &IGame::resetLoadingBar));
	ResourceManager->notify_progress.connect(sigc::mem_fun(this, &IGame::notifyLoadingBar));

	LOG_DEBUG(("initializing resource manager..."));
	ResourceManager->init(data_dir + "/resources.xml");
	
	if (_show_fps) {
		LOG_DEBUG(("creating `digits' object..."));
		_fps = ResourceManager->createObject("damage-digits", "damage-digits");
		_fps->onSpawn();
		_fps->speed = 0;
	} else _fps = NULL;

	
/*	
	if (_preload_map.size()) {
		LOG_DEBUG(("starting predefined map %s...", _preload_map.c_str()));
		loadMap(_preload_map);
		
		_my_index = PlayerManager->spawnPlayer("shilka", "green-shilka", "keys");
		assert(_my_index == 0);
		PlayerManager->spawnPlayer("ai-tank", "green-tank", "ai");
		PlayerManager->setViewport(_my_index, _window.getSize());
		_main_menu.setActive(false);
	}
*/
	if (_autojoin) {
		onMenu("m-join", address);
		_main_menu.setActive(false);
	}
	
}

bool IGame::onKey(const SDL_keysym key) {
	if (key.sym == SDLK_ESCAPE) {
		if (!_map_loaded) {
			_main_menu.setActive(true);
			return true;
		}
		
		LOG_DEBUG(("escape hit, paused: %s", _paused?"true":"false"));
		_main_menu.setActive(!_main_menu.isActive());
		if (PlayerManager->isServer() || PlayerManager->isClient()) {
			_paused = false;
		} else {
			_paused = _main_menu.isActive();
		}
		return true;
	}

	return false;
}

void IGame::onMenu(const std::string &name, const std::string &value) {
	if (name == "quit") 
		_running = false;
	else if (name == "start") {
		const std::string &vehicle = value;
		LOG_DEBUG(("start single player as '%s' requested", vehicle.c_str()));

		clear();
		_main_menu.reset();
		_cheater = new Cheater;
		
		throw_ex(("reimplement me"));
		
	} else if (name == "s-start") {
		LOG_DEBUG(("start split screen game requested"));
		clear();
		_main_menu.reset();
		std::string vehicle1, vehicle2;
		Config->get("menu.default-vehicle-1", vehicle1, "launcher");
		Config->get("menu.default-vehicle-2", vehicle2, "launcher");
		std::string map;
		Config->get("menu.default-mp-map", map, "survival");
		loadMap(map);

		static const char * colors[4] = {"green", "red", "yellow", "cyan"};
		std::string animation1 = colors[mrt::random(4)];
		std::string animation2 = colors[mrt::random(4)];
		animation1 += "-" + vehicle1;
		animation2 += "-" + vehicle2;

		GET_CONFIG_VALUE("player.control-method-1", std::string, cm, "keys-1");		
		GET_CONFIG_VALUE("player.control-method-2", std::string, cm2, "keys-2");
		
		_my_index = PlayerManager->spawnPlayer(vehicle1, animation1, cm);
		assert(_my_index == 0);
		PlayerManager->spawnPlayer(vehicle2, animation2, cm2);
		
		v3<int> ts = Map->getTileSize();
		int w = _window.getSize().w / 2;

		sdlx::Rect vp1(_window.getSize());
		sdlx::Rect vp2(_window.getSize());
		vp1.w = w;

		vp2.x = w;
		vp2.w = w;
		LOG_DEBUG(("p1: %d %d %d %d", vp1.x, vp1.y, vp1.w, vp1.h));
		LOG_DEBUG(("p2: %d %d %d %d", vp2.x, vp2.y, vp2.w, vp2.h));
		PlayerManager->setViewport(0, vp1);
		PlayerManager->setViewport(1, vp2);
		
		_cheater = new Cheater;

	} else if (name == "m-start") {
		LOG_DEBUG(("start multiplayer server requested"));
		clear();
		std::string map;
		Config->get("menu.default-mp-map", map, "survival");
		loadMap(map);

		GET_CONFIG_VALUE("player.control-method", std::string, cm, "keys");

		std::string vehicle, animation;
		PlayerManager->getDefaultVehicle(vehicle, animation);
		_my_index = PlayerManager->spawnPlayer(vehicle, animation, cm);

		assert(_my_index == 0);

		PlayerManager->setViewport(_my_index, _window.getSize());
		PlayerManager->startServer();
	} else if (name == "m-join") {
		clear();
		Config->set("multiplayer.recent-host", value);
		PlayerManager->startClient(value);
		
		_main_menu.setActive(false);
	} else if (name == "credits" && !PlayerManager->isServer()) {
		LOG_DEBUG(("show credits."));
		_credits = new Credits;
	}
}

void IGame::stopCredits() {
	delete _credits;
	_credits = NULL;
	
	t_start = SDL_GetTicks();
}



void IGame::loadMap(const std::string &name, const bool spawn_objects) {
	_main_menu.setActive(false);
	IMap &map = *IMap::get_instance();
	map.load(name);
	_waypoints.clear();
	_waypoint_edges.clear();
	
	Config->clearOverrides();
	
	//const v3<int> size = map.getSize();
	for (IMap::PropertyMap::iterator i = map.properties.begin(); i != map.properties.end(); ++i) {
		std::vector<std::string> res;
		mrt::split(res, i->first, ":");
		const std::string &type = res[0];
		
		if (type != "spawn" && type != "object" && type != "waypoint" && 
			type != "edge" && type != "config")
			throw_ex(("unsupported line: '%s'", i->first.c_str()));
		
		if (!spawn_objects && type != "waypoint" && type != "edge")
			continue;
	
		v3<int> pos;
		if (type != "edge" && type != "config") {
			std::string pos_str = i->second;
			const bool tiled_pos = pos_str[0] == '@';
			if (tiled_pos) { 
				pos_str = pos_str.substr(1);
			}
			TRY {
				pos.fromString(pos_str);
			} CATCH(mrt::formatString("parsing '%s'=>'%s'", i->first.c_str(), i->second.c_str()).c_str() , throw;)
			if (tiled_pos) {
				v3<int> tile_size = Map->getTileSize();
				pos.x *= tile_size.x;
				pos.y *= tile_size.y;
				//keep z untouched.
			}
		}
	
		/*
		if (pos.x < 0) 
			pos.x += size.x;
		if (pos.y < 0) 
			pos.y += size.y;
		*/
		
		if (type == "spawn") {
			LOG_DEBUG(("spawnpoint: %d,%d", pos.x, pos.y));
			PlayerManager->addSlot(pos);
		} else {
			if (type == "object") {
				//LOG_DEBUG(("object %s, animation %s, pos: %s", res[1].c_str(), res[2].c_str(), i->second.c_str()));
				if (res.size() < 3)
					throw_ex(("'%s' misses an argument", i->first.c_str()));
				Item item;
				Object *o = ResourceManager->createObject(res[1], res[2]);
				o->addOwner(-42); //fake owner ;)
				World->addObject(o, pos.convert<float>());
				
				item.classname = res[1];
				item.animation = res[2];
				item.position = pos;
				item.dead_on = 0;
				item.destroy_for_victory = res[3].substr(0, 19) == "destroy-for-victory";
				if (item.destroy_for_victory) {
					LOG_DEBUG(("%s:%s critical for victory", res[1].c_str(), res[2].c_str()));
				}
				
				item.id = o->getID();
				_items.push_back(item);
			} else if (type == "waypoint") {
				if (res.size() < 3)
					throw_ex(("'%s' misses an argument", i->first.c_str()));
				LOG_DEBUG(("waypoint class %s, name %s : %d,%d", res[1].c_str(), res[2].c_str(), pos.x, pos.y));
				_waypoints[res[1]][res[2]] =  pos;
			} else if (type == "edge") {
				if (res.size() < 3)
					throw_ex(("'%s' misses an argument", i->first.c_str()));
				if (res[1] == res[2])
					throw_ex(("map contains edge from/to the same vertex"));
				_waypoint_edges.insert(WaypointEdgeMap::value_type(res[1], res[2]));
			} else if (type == "config") {
				if (res.size() < 2)
					throw_ex(("'%s' misses an argument", i->first.c_str()));
				
				std::vector<std::string> value;
				mrt::split(value, i->second, ":");
				if (value[0] != "int" && value[0] != "float" && value[0] != "string")
					throw_ex(("cannot set config variable '%s' of type '%s'", res[1].c_str(), value[0].c_str()));
				Var var(value[0]);
				var.fromString(value[1]);

				Config->setOverride(res[1], var);
			}
		}
	}
	LOG_DEBUG(("checking waypoint graph..."));
	for(WaypointEdgeMap::const_iterator i = _waypoint_edges.begin(); i != _waypoint_edges.end(); ++i) {
		const std::string &dst = i->second;
		WaypointEdgeMap::const_iterator b = _waypoint_edges.lower_bound(dst);
		if (b == _waypoint_edges.end() || b->first != dst)
			throw_ex(("no edges out of waypoint '%s'", dst.c_str()));
	}
	LOG_DEBUG(("%u items on map. %u waypoints, %u edges", (unsigned) _items.size(), (unsigned)_waypoints.size(), (unsigned)_waypoint_edges.size()));
	Config->invalidateCachedValues();
	
	_hud->initMap();
	
	_map_loaded = true;
	_game_over = false;
	t_start = SDL_GetTicks();
}

void IGame::gameOver(const std::string &state, const float time) {
	_game_over = true;
	displayMessage(state, time);
	PlayerManager->gameOver(state, time);
}

void IGame::displayMessage(const std::string &message, const float time) {
	if (_hud == NULL)
		throw(("hud was not initialized"));
	_hud->pushState(message, time);
}


void IGame::checkItems(const float dt) {
	std::string game_state = _hud->popState(dt);
	if (_game_over && !game_state.empty()) {
		clear();
	}
	
	if (_game_over || !_check_items.tick(dt) || PlayerManager->isClient()) //no need for multiplayer.
		return;
	
	int goal = 0, goal_total = 0;
	
	for(Items::iterator i = _items.begin(); i != _items.end(); ++i) {
		Item &item = *i;
		if (item.destroy_for_victory)
			++goal_total;
		Object *o = World->getObjectByID(item.id);
		if (o != NULL) {
			if (o->getState() == "broken") 
				++goal;
			continue;
		}
		if (item.destroy_for_victory)
			++goal;
		
		Uint32 ticks = SDL_GetTicks();
		if (item.dead_on == 0) {
			item.dead_on = ticks;
			LOG_DEBUG(("item %d:%s:%s is dead, log dead time.", item.id, item.classname.c_str(), item.animation.c_str()));
			continue;
		}
		int rt;
		Config->get("map." + item.classname + ".respawn-interval", rt, 5); 
		if (rt < 0) 
			continue;
		if (((ticks - item.dead_on) / 1000) >= (unsigned)rt) {
			//respawning item
			LOG_DEBUG(("respawning item: %s:%s", item.classname.c_str(), item.animation.c_str()));
			Object *o = ResourceManager->createObject(item.classname, item.animation);
			o->addOwner(-42);
			World->addObject(o, item.position.convert<float>());
			item.id = o->getID();
			item.dead_on = 0;
		}
	}
	if (goal_total > 0 && goal == goal_total) {
		gameOver("YOU WIN", 5);
	}
}

void IGame::run() {
	LOG_DEBUG(("entering main loop"));
	SDL_Event event;

	sdlx::Rect window_size = _window.getSize();
	
	GET_CONFIG_VALUE("engine.fps-limit", int, fps_limit, 1000);
	
	float fr = fps_limit;
	int max_delay = 1000/fps_limit;
	LOG_DEBUG(("fps_limit set to %d, maximum frame delay: %d", fps_limit, max_delay));

	while (_running) {
		t_start  = SDL_GetTicks();
#ifdef SHOW_PERFSTATS
		Uint32 t_tick_n = t_start, t_tick_w = t_start, t_tick_s = t_start, t_tick_c = t_start;
#endif

		
		while (SDL_PollEvent(&event)) {
			switch(event.type) {
			case SDL_KEYDOWN:
#ifndef WIN32
				if (event.key.keysym.sym==SDLK_RETURN && event.key.keysym.mod & KMOD_CTRL) {
					TRY {
						_window.toggleFullscreen();
					} CATCH("main loop", {});
					break;
				}
#endif
				if (event.key.keysym.sym==SDLK_s && event.key.keysym.mod & KMOD_SHIFT) {
					_window.saveBMP("screenshot.bmp");
					break;
				}
				if (event.key.keysym.sym==SDLK_m && event.key.keysym.mod & KMOD_SHIFT && _map_loaded) {
					const v3<int> msize = Map->getSize();
					LOG_DEBUG(("creating map screenshot %dx%d", msize.x, msize.y));

					sdlx::Surface screenshot;
					screenshot.createRGB(msize.x, msize.y, 32, SDL_SWSURFACE | SDL_SRCALPHA);
					screenshot.convertAlpha();
					screenshot.fillRect(screenshot.getSize(), screenshot.mapRGBA(0,0,0,255));

					sdlx::Rect viewport(0, 0, msize.x, msize.y);
					World->render(screenshot, viewport, viewport);
					screenshot.saveBMP("map.bmp"); //hopefully we're done here.
					break;
				}
				if (!PlayerManager->isClient() && event.key.keysym.sym==SDLK_F12 && _my_index >= 0) {
					PlayerSlot &slot = PlayerManager->getSlot(_my_index);
					if (slot.frags > 0) 
						--slot.frags;

					Object *o = slot.getObject();
					if (o)
						o->emit("death", 0);
					break;
				}
				if (_credits) {
					stopCredits();
					break;
				}
				key_signal.emit(event.key.keysym);
			break;
			case SDL_MOUSEBUTTONDOWN:
				if (_credits) {
					stopCredits();
				}
			case SDL_MOUSEBUTTONUP:
				{
					int bi = (event.button.button == SDL_BUTTON_LEFT)? 0: 
						((event.button.button == SDL_BUTTON_RIGHT)?1:
						((event.button.button == SDL_BUTTON_MIDDLE)?2:-1));
					mouse_signal.emit(bi, event.button.type == SDL_MOUSEBUTTONDOWN, event.button.x, event.button.y);
				}
				break;
		    case SDL_QUIT:
				_running = false;
			break;
    		}
		}

		
		const float dt = 1.0/fr;
		
		if (_map_loaded && _credits == NULL && _running && !_paused) {
			PlayerManager->updatePlayers();
			
			checkItems(dt);
#ifdef SHOW_PERFSTATS
			t_tick_n = SDL_GetTicks();
#endif
		
			World->tick(dt);
			Mixer->updateObjects();
#ifdef SHOW_PERFSTATS
			t_tick_w = SDL_GetTicks();
#endif

			
		}
	
		if (_running && !_paused)
			PlayerManager->tick(t_start, dt);

#ifdef SHOW_PERFSTATS
		Uint32 t_tick = SDL_GetTicks();
#endif
		
		if (_credits || _map_loaded)
			_window.fillRect(window_size, 0);
		else _hud->renderSplash(_window);
		
		int vx = 0, vy = 0;

		if (_credits) {
			_credits->render(dt, _window);
			goto flip;
		}
	
		if (_shake > 0) {
			vy += _shake_int;
		}		

		PlayerManager->render(_window, vx, vy);
		
		if (_shake > 0) {
			_shake_int = -_shake_int;
			_shake -= dt;
		}
		
		if (_map_loaded) {
			_hud->render(_window);
			_hud->renderRadar(dt, _window);
		}

		_main_menu.render(_window);
		Console->render(_window);
		
flip:
		if (_show_fps) {
			_fps->hp = (int)fr;
			_fps->render(_window, _window.getWidth() - (int)(_fps->size.x * 3), 0);
		}

		
#ifdef SHOW_PERFSTATS
		Uint32 t_render = SDL_GetTicks();
#endif

		Window::flip();

#ifdef SHOW_PERFSTATS
		Uint32 t_flip = SDL_GetTicks();
#endif

		int tdelta = SDL_GetTicks() - t_start;

#ifdef SHOW_PERFSTATS
		LOG_DEBUG(("tick time: %u, render time: %u, flip time: %u", t_tick - t_start, t_render - t_tick, t_flip - t_render));
		LOG_DEBUG(("notify: %u, world: %u, server: %u, client: %u", t_tick_n - t_start, t_tick_w - t_tick_n, t_tick_s - t_tick_w, t_tick_c - t_tick_s));
#endif
		if (tdelta < max_delay) {
#ifdef SHOW_PERFSTATS
			LOG_DEBUG(("tdelta: %d, delay: %d", tdelta, max_delay - tdelta));
#endif
			SDL_Delay(max_delay - tdelta);
		}

		tdelta = SDL_GetTicks() - t_start;
		fr = (tdelta != 0)? (1000.0 / tdelta): 1000;
	}
	LOG_DEBUG(("exiting main loop."));
	if (_running)
		throw_sdl(("SDL_WaitEvent"));
}

void IGame::deinit() {
	clear();
	Config->save();
	
	delete _fps;
	_fps = NULL;
	
	delete _hud;
	_hud = NULL;
	
	_running = false;
	_map_loaded = false;
	Window::deinit();
	
	_main_menu.deinit();

	delete _credits;
	_credits = NULL;	
}



void IGame::clear() {
	LOG_DEBUG(("cleaning up main game object..."));
	Mixer->cancelAll();

	PlayerManager->clear();

	_my_index = -1;
	_items.clear();
	_waypoints.clear();
	_waypoint_edges.clear();
	World->clear();
	_paused = false;
	_map_loaded = false;
	_game_over = false;
	Map->clear();
	
	delete _credits;
	_credits = NULL;
	
	delete _cheater;
	_cheater = NULL;

	_main_menu.setActive(true);
}


void IGame::shake(const float duration, const int intensity) {
	_shake = duration;
	_shake_int = intensity;
}

void IGame::resetLoadingBar(const int total) {
	_loading_bar_now = 0;
	_loading_bar_total = total;
}

void IGame::notifyLoadingBar(const int progress) {
	GET_CONFIG_VALUE("hud.disable-loading-screen", bool, disable_bar, false);
	if (disable_bar)
		return;
	
	float old_progress = 1.0 * _loading_bar_now / _loading_bar_total;
	_loading_bar_now += progress;
	
	_window.fillRect(_window.getSize(), 0);
	//_window.fillRect(_window.getSize(), _window.mapRGB(255, 255, 255));

	if (_hud->renderLoadingBar(_window, old_progress, 1.0 * _loading_bar_now / _loading_bar_total))
		_window.flip();
}

const std::string IGame::getRandomWaypoint(const std::string &classname, const std::string &last_wp) const {
	if (last_wp.empty()) 
		throw_ex(("getRandomWaypoint('%s', '%s') called with empty name", classname.c_str(), last_wp.c_str()));
	
	WaypointClassMap::const_iterator wp_class = _waypoints.find(classname);
	if (wp_class == _waypoints.end()) 
		throw_ex(("no waypoints for '%s' defined", classname.c_str()));
		
	WaypointEdgeMap::const_iterator b = _waypoint_edges.lower_bound(last_wp);
	WaypointEdgeMap::const_iterator e = _waypoint_edges.upper_bound(last_wp);
	if (b == e) 
		throw_ex(("no edges defined for waypoint '%s'", last_wp.c_str()));

	int wp = mrt::random(_waypoint_edges.size() * 2);
	while(true) {
		for(WaypointEdgeMap::const_iterator i = b; i != e; ++i) {
			if (wp-- <= 0) {
				return i->second;
			}
		}
	}
	throw_ex(("getRandomWaypoint(unexpected termination)"));
	return "*bug*";
}

const std::string IGame::getNearestWaypoint(const BaseObject *obj, const std::string &classname) const {
	v3<int> pos;
	obj->getPosition(pos);
	int distance = -1;
	std::string wp;
	
	WaypointClassMap::const_iterator i = _waypoints.find(classname);
	if (i == _waypoints.end())
		throw_ex(("no waypoints for '%s' found", classname.c_str()));

	for(WaypointMap::const_iterator j = i->second.begin(); j != i->second.end(); ++j) {
		int d = j->second.quick_distance(pos);
		if (distance == -1 || d < distance) {
			distance = d;
			wp = j->first;
		}
	}
	return wp;
}


void IGame::getWaypoint(v3<float> &wp, const std::string &classname, const std::string &name) {
	if (name.empty() || classname.empty()) 
		throw_ex(("getWaypoint('%s', '%s') called with empty classname and/or name", classname.c_str(), name.c_str()));
	
	WaypointClassMap::const_iterator wp_class = _waypoints.find(classname);
	if (wp_class == _waypoints.end()) 
		throw_ex(("no waypoints for '%s' defined", classname.c_str()));
	
	WaypointMap::const_iterator i = wp_class->second.find(name);
	if (i == wp_class->second.end())
		throw_ex(("no waypoints '%s' defined", name.c_str()));
	wp = i->second.convert<float>();
}

const std::string IGame::onConsole(const std::string &cmd, const std::string &param) {
	if (cmd == "quit") {
		_running = false;
		return "thank you for playing battle tanks";
	} else if (cmd == "spawnplayer") {
		std::vector<std::string> par;
		mrt::split(par, param, " ", 3);
		if (par.size() < 3 || par[0].empty() || par[1].empty() || par[2].empty())
			return "usage: spawnPlayer object animation control-method";
		try {
			PlayerManager->spawnPlayer(par[0], par[1], par[2]);
		} catch(const std::exception &e) {
			return std::string("error: ") + e.what();
		}
		return "ok";
	}
	return std::string();
}
