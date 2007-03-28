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

#include "tmx/map.h"

#include "mrt/logger.h"
#include "mrt/exception.h"
#include "mrt/random.h"
#include "mrt/fs_node.h"

#include "sdlx/system.h"
#include "sdlx/sdl_ex.h"
#include "sdlx/joystick.h"
#include "sdlx/ttf.h"
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

#include "math/v3.h"
#include "menu/menu.h"
#include "i18n.h"
#include <math.h>

IMPLEMENT_SINGLETON(Game, IGame)

IGame::IGame() : _main_menu(NULL),
_check_items(0.5, true),  _autojoin(false), _shake(0), _show_radar(true) , _show_stats(false), _credits(NULL), _cheater(NULL), _state_timer(false){}
IGame::~IGame() {}

void IGame::resetTimer() {
	_timer.reset();
}

void IGame::init(const int argc, char *argv[]) {
	srand(time(NULL));
	
	Config->load("bt.xml");

	
	{
		//setup some defaults
		
		int r;
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
			Window::init(argc, argv);
			exit(0);
		}

	}
	
	I18n->load(Finder->find("strings.xml"), lang);
	
	
	Window::init(argc, argv);
	Mixer->init(no_sound, no_music);
	
	Mixer->loadPlaylist(Finder->find("playlist"));
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
	
	Console->init();
	Console->on_command.connect(sigc::mem_fun(this, &IGame::onConsole));

	if (_main_menu == NULL) {
		LOG_DEBUG(("initializing menus..."));		
		_main_menu = new MainMenu(_window.getWidth(), _window.getHeight());
	}

	_paused = false;
	_running = true;
	_map_loaded = false;

	_window.fillRect(_window.getSize(), 0);
	_window.flip();
	
	LOG_DEBUG(("initializing hud..."));
	_hud = new Hud(_window.getWidth(), _window.getHeight());
	_big_font = ResourceManager->loadFont("big", true);
	

	LOG_DEBUG(("installing callbacks..."));
	key_signal.connect(sigc::mem_fun(this, &IGame::onKey));
	_main_menu->menu_signal.connect(sigc::mem_fun(this, &IGame::onMenu));
	
	Map->reset_progress.connect(sigc::mem_fun(this, &IGame::resetLoadingBar));
	Map->notify_progress.connect(sigc::mem_fun(this, &IGame::notifyLoadingBar));
	ResourceManager->reset_progress.connect(sigc::mem_fun(this, &IGame::resetLoadingBar));
	ResourceManager->notify_progress.connect(sigc::mem_fun(this, &IGame::notifyLoadingBar));

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
		if (_main_menu)
			_main_menu->setActive(false);
	}
}

bool IGame::onKey(const SDL_keysym key) {
	if (key.sym == SDLK_ESCAPE) {
		if (!_map_loaded) {
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

void IGame::onMenu(const std::string &name, const std::string &value) {
	if (name == "quit") 
		_running = false;
	else if (name == "start") {
		const std::string &vehicle = value;
		LOG_DEBUG(("start single player as '%s' requested", vehicle.c_str()));

		clear();
		if (_main_menu)
			_main_menu->reset();
		_cheater = new Cheater;
		
		throw_ex(("reimplement me"));
		
	} else if (name == "s-start") {
		LOG_DEBUG(("start split screen game requested"));
		clear();
		if (_main_menu)
			_main_menu->reset();
		std::string vehicle1, vehicle2, animation1, animation2;
		Config->get("menu.default-vehicle-1", vehicle1, "launcher");
		Config->get("menu.default-vehicle-2", vehicle2, "launcher");
		std::string map;
		Config->get("menu.default-mp-map", map, "survival");
		loadMap(map);

		GET_CONFIG_VALUE("player.control-method-1", std::string, cm, "keys-1");		
		GET_CONFIG_VALUE("player.control-method-2", std::string, cm2, "keys-2");

		PlayerManager->getDefaultVehicle(vehicle1, animation1);
		PlayerManager->getDefaultVehicle(vehicle2, animation2);
		
		PlayerManager->spawnPlayer(vehicle1, animation1, cm);
		PlayerManager->spawnPlayer(vehicle2, animation2, cm2);
		
		v2<int> ts = Map->getTileSize();
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
		int idx = PlayerManager->spawnPlayer(vehicle, animation, cm);

		assert(idx == 0);

		PlayerManager->setViewport(idx, _window.getSize());
		PlayerManager->startServer();
	} else if (name == "m-join") {
		clear();
		std::string address;
		Config->get("multiplayer.recent-host", address, "LOCALHOST");
		bool ok = true;
		TRY {
			PlayerManager->startClient(address);
		} CATCH("startClient", { displayMessage("errors", "connection-failed", 1); ok = false; });
		
		if (_main_menu)	
			_main_menu->setActive(!ok);
	} else if (name == "credits" && !PlayerManager->isServer()) {
		LOG_DEBUG(("show credits."));
		_credits = new Credits;
	}
}

void IGame::stopCredits() {
	delete _credits;
	_credits = NULL;
	
	resetTimer();
}


template<typename T>
static void coord2v(T &pos, const std::string &str) {
	std::string pos_str = str;

	const bool tiled_pos = pos_str[0] == '@';
	if (tiled_pos) { 
		pos_str = pos_str.substr(1);
	}

	TRY {
		pos.fromString(pos_str);
	} CATCH(mrt::formatString("parsing '%s'", str.c_str()).c_str() , throw;)

	if (tiled_pos) {
		v2<int> tile_size = Map->getTileSize();
		pos.x *= tile_size.x;
		pos.y *= tile_size.y;
		//keep z untouched.
	}
}


void IGame::loadMap(const std::string &name, const bool spawn_objects) {
	if (_main_menu)
		_main_menu->setActive(false);
	IMap &map = *IMap::get_instance();
	map.load(name);

	World->initMap();

	_waypoints.clear();
	_waypoint_edges.clear();
	
	Config->clearOverrides();
	
	//const v2<int> size = map.getSize();
	for (IMap::PropertyMap::iterator i = map.properties.begin(); i != map.properties.end(); ++i) {
		std::vector<std::string> res;
		mrt::split(res, i->first, ":");
		const std::string &type = res[0];
		
		if (type != "spawn" && type != "object" && type != "waypoint" && 
			type != "edge" && type != "config" && type != "checkpoint" &&
			type != "hint")
			throw_ex(("unsupported line: '%s'", i->first.c_str()));
		
		if (!spawn_objects && type != "waypoint" && type != "edge" && type != "config")
			continue;
	
		v3<int> pos;
		if (type != "edge" && type != "config") {
			coord2v< v3<int> >(pos, i->second);
		}
	
		/*
		if (pos.x < 0) 
			pos.x += size.x;
		if (pos.y < 0) 
			pos.y += size.y;
		*/
		
		if (type == "spawn") {
			LOG_DEBUG(("spawnpoint: %d,%d,%d", pos.x, pos.y, pos.z));
			v2<int> tile_size = Map->getTileSize();
			pos.x += tile_size.x / 2;
			pos.y += tile_size.y / 2;
			PlayerManager->addSlot(pos);
		} else {
			if (type == "object") {
				//LOG_DEBUG(("object %s, animation %s, pos: %s", res[1].c_str(), res[2].c_str(), i->second.c_str()));
				if (res.size() < 3)
					throw_ex(("'%s' misses an argument", i->first.c_str()));
				Item item;
				Object *o = ResourceManager->createObject(res[1], res[2]);

				if (pos.z)
					o->setZ(pos.z, true);
				
				o->addOwner(-42); //fake owner ;)
				World->addObject(o, v2<float>(pos.x, pos.y));
				
				item.classname = res[1];
				item.animation = res[2];
				item.position = v2<int>(pos.x, pos.y);
				item.z = pos.z;
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
				_waypoints[res[1]][res[2]] = v2<int>(pos.x, pos.y);
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
			} else if (type == "checkpoint" || type == "hint") {
				LOG_DEBUG(("%s %s %s", type.c_str(), i->first.c_str(), i->second.c_str()));
				std::vector<std::string> value;
				mrt::split(value, i->second, ":");
				if (value.size() < 2)
					throw_ex(("'%s' misses an argument", i->first.c_str()));
				v3<int> pos;
				v2<int> size;
				coord2v(pos, value[0]);
				coord2v(size, value[1]);
				if (type == "checkpoint") {
					PlayerManager->addCheckpoint(pos, size, i->first.substr(11));
				} else if (type == "hint") {
					if (res.size() < 2)
						throw_ex(("hint must contain name."));
					PlayerManager->addHint(pos, size, "hints/" + name, res[1]);
				} else assert(0);
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
	
	GET_CONFIG_VALUE("engine.max-time-slice", float, mts, 0.025);
	World->setTimeSlice(mts);
	
	_map_loaded = true;
	_game_over = false;
	
	delete _cheater;
	_cheater = NULL;
	if (!PlayerManager->isClient())
		_cheater = new Cheater; 
	
	resetTimer();
}

void IGame::gameOver(const std::string &area, const std::string &message, const float time) {
	_game_over = true;
	displayMessage(area, message, time);
	PlayerManager->gameOver(message, time);
}

void IGame::displayMessage(const std::string &area, const std::string &message, const float time) {
	pushState(I18n->get(area, message), time);
}


void IGame::checkItems(const float dt) {	
	if (!_map_loaded || _paused || _game_over || !_check_items.tick(dt) || PlayerManager->isClient()) //no need for multiplayer.
		return;
	
	int goal = 0, goal_total = 0;
	
	_specials.clear();
	
	for(Items::iterator i = _items.begin(); i != _items.end(); ++i) {
		Item &item = *i;
		if (item.destroy_for_victory)
			++goal_total;
		Object *o = World->getObjectByID(item.id);
		if (o != NULL) {
			if (item.destroy_for_victory) {
				if (o->getState() == "broken") {
					++goal;
				} else 
					_specials.push_back(item.position);
			} 
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
			if (item.z) 
				o->setZ(item.z, true);
			o->addOwner(-42);
			
			World->addObject(o, item.position.convert<float>());
			item.id = o->getID();
			item.dead_on = 0;
		}
	}
	if (goal_total > 0 && goal == goal_total) {
		gameOver("messages", "mission-accomplished", 5);
	}
}

void IGame::run() {
	LOG_DEBUG(("entering main loop"));
	SDL_Event event;

	sdlx::Rect window_size = _window.getSize();
	
	GET_CONFIG_VALUE("engine.fps-limit", int, fps_limit, 1000);
	
	float fr = fps_limit;
	int max_delay = 1000000 / fps_limit;
	LOG_DEBUG(("fps_limit set to %d, maximum frame delay: %d", fps_limit, max_delay));

	while (_running) {
		_timer.reset();
		
		while (SDL_PollEvent(&event)) {
			switch(event.type) {
			case SDL_KEYUP:
				if (event.key.keysym.sym == SDLK_TAB) {
					_show_stats = false;
				}
			break;
			
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
					static int n = 0; 
					std::string fname;
					do {
						fname = mrt::formatString("screenshot%02d.bmp", n++);
					} while(mrt::FSNode::exists(fname));
					LOG_DEBUG(("saving screenshot to %s", fname.c_str()));
					_window.saveBMP(fname);
					break;
				}
				if (event.key.keysym.sym==SDLK_m && event.key.keysym.mod & KMOD_SHIFT && _map_loaded) {
					const v2<int> msize = Map->getSize();
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
				if (event.key.keysym.sym == SDLK_m && !_main_menu->isActive()) {
					_show_radar = !_show_radar;
					break;
				}
				if (event.key.keysym.sym == SDLK_TAB) {
					_show_stats = true;
				}

				if (!PlayerManager->isClient() && event.key.keysym.sym==SDLK_F12 && PlayerManager->getSlotsCount() > 0) {
					PlayerSlot &slot = PlayerManager->getSlot(0);
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
					break;
				}
			case SDL_MOUSEBUTTONUP:
				{
					mouse_signal.emit(event.button.button, event.button.type == SDL_MOUSEBUTTONDOWN, event.button.x, event.button.y);
				}
				break;
		    case SDL_QUIT:
				_running = false;
			break;
    		}
		}

		
		const float dt = 1.0/fr;
		
		if (_running && !_paused) {
			std::string game_state = popState(dt);
			if (_game_over && !game_state.empty()) {
				clear();
			}

			PlayerManager->tick(SDL_GetTicks(), dt);
		}

		if (_map_loaded && _credits == NULL && _running && !_paused) {
			checkItems(dt);
			PlayerManager->updatePlayers();
			
			Map->tick(dt);
			World->tick(dt);
			Mixer->updateObjects();
		}

		
		if (_main_menu)
			_main_menu->tick(dt);

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

			if (_show_radar) {
				_hud->renderRadar(dt, _window, _specials);
			}
			if (_main_menu && !_main_menu->isActive() && _show_stats) {
				_hud->renderStats(_window);
			}
		}

		if (_main_menu)
			_main_menu->render(_window);
		
		if (!_state.empty()) {
			int w = _big_font->render(NULL, 0, 0, _state);
			int x = (_window.getWidth() - w) / 2;
			int y = (_window.getHeight() - _big_font->getHeight()) / 2;
			
			_big_font->render(_window, x, y, _state);
		}
		
		Console->render(_window);
		
flip:
		if (_show_fps) {
			_fps->hp = (int)fr;
			_fps->render(_window, _window.getWidth() - (int)(_fps->size.x * 3), 0);
		}
		if (_show_log_lines) {
			_log_lines->hp = mrt::Logger->getLinesCounter();
			int size = (_log_lines->hp > 0)? (int)log10((double)_log_lines->hp) + 2:2;
			_log_lines->render(_window, _window.getWidth() - (int)(_log_lines->size.x * size), 20);
		}

		
		Window::flip();

		int t_delta = _timer.microdelta();

		if (t_delta < max_delay) {
			//LOG_DEBUG(("tdelta: %d, delay: %d", t_delta, max_delay - t_delta));
			sdlx::Timer::microsleep(max_delay - t_delta);
		}

		t_delta = _timer.microdelta();
		fr = (t_delta != 0)? (1000000.0 / t_delta): 1000000;
	}
	LOG_DEBUG(("exiting main loop."));
	if (_running)
		throw_sdl(("SDL_WaitEvent"));
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
	
	_running = false;
	_map_loaded = false;
	Window::deinit();
	
	if (_main_menu)
		_main_menu->deinit();

	delete _credits;
	_credits = NULL;	

	Config->save();
}



void IGame::clear() {
	LOG_DEBUG(("cleaning up main game object..."));
	Mixer->cancelAll();

	PlayerManager->clear();

	_items.clear();
	_waypoints.clear();
	_waypoint_edges.clear();
	World->clear();
	_paused = false;
	_map_loaded = false;
	_game_over = false;
	_show_radar = true;
	_show_stats = false;
	Map->clear();
	
	delete _credits;
	_credits = NULL;
	
	delete _cheater;
	_cheater = NULL;

	if (_main_menu)
		_main_menu->setActive(true);
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
	v2<int> pos;
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


void IGame::getWaypoint(v2<float> &wp, const std::string &classname, const std::string &name) {
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
try {
	if (cmd == "quit") {
		_running = false;
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
			if (!_map_loaded)
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
			o->addOwner(-42);
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
		LOG_NOTICE(("%g %g", o->getPosition().x, o->getPosition().y));
		return mrt::formatString("%g %g", o->getPosition().x, o->getPosition().y);
	}

} catch(const std::exception &e) {
	return std::string("error: ") + e.what();
}
	return std::string();
}

void IGame::pushState(const std::string &state, const float time) {
	_state = state;
	_state_timer.set(time);
}

const std::string IGame::popState(const float dt) {
	if (_state.empty() || !_state_timer.tick(dt))
		return std::string();
	std::string r = _state;
	_state.clear();
	return r;
}
