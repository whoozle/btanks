/* Battle Tanks Game
 * Copyright (C) 2006 Battle Tanks team
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

#include <SDL/SDL_opengl.h>
#include <SDL/SDL_net.h>

#include "controls/joyplayer.h"
#include "controls/keyplayer.h"
#include "controls/mouse_control.h"
#include "controls/external_control.h"

#include "player_state.h"
#include "config.h"

#include "sound/mixer.h"

#ifndef SDL_OPENGLBLIT
#define SDL_OPENGLBLIT 0
// using 0 as OPENGLBLIT value. SDL 1.3 or later
#endif

//#define SHOW_PERFSTATS

IMPLEMENT_SINGLETON(Game, IGame)

IGame::IGame() : 
_check_items(0.5, true), _my_index(-1), _address("localhost"), _autojoin(false), _shake(0), _trip_time(10), _next_sync(true) {
	//LOG_DEBUG(("IGame ctor"));
}
IGame::~IGame() {}

void IGame::init(const int argc, char *argv[]) {
	srand(time(NULL));
	
	_server = NULL; _client = NULL;
	_ping = false;
#ifdef __linux__
//	putenv("SDL_VIDEODRIVER=dga");
#endif

	_opengl = true;
	
	Config->load("bt.xml");
	GET_CONFIG_VALUE("engine.show-fps", bool, show_fps, true);
	GET_CONFIG_VALUE("engine.data-directory", std::string, data_dir, "data");
	
	_show_fps = show_fps;
	bool fullscreen = false;
	bool dx = false;
	bool vsync = false;

	GET_CONFIG_VALUE("engine.sound.disable-sound", bool, no_sound, false);
	GET_CONFIG_VALUE("engine.sound.disable-music", bool, no_music, false);

	int w = 800, h = 600;
	
	for(int i = 1; i < argc; ++i) {
		if (strcmp(argv[i], "--no-gl") == 0) _opengl = false;
		else if (strcmp(argv[i], "--fs") == 0) fullscreen = true;
		else if (strcmp(argv[i], "--vsync") == 0) vsync = true;
#ifdef WIN32
		else if (strcmp(argv[i], "--dx") == 0) { dx = true; _opengl = false; }
#endif
		else if (strcmp(argv[i], "-0") == 0) { w = 640; h = 480; }
		else if (strcmp(argv[i], "-1") == 0) { w = 800; h = 600; }
		else if (strcmp(argv[i], "-2") == 0) { w = 1024; h = 768; }
		else if (strcmp(argv[i], "-3") == 0) { w = 1280; h = 1024; }
		else if (strncmp(argv[i], "--map=", 6) == 0) { _preload_map = argv[i] + 6; }
		else if (strncmp(argv[i], "--connect=", 10) == 0) { _address = argv[i] + 10; _autojoin = true; }
		else if (strcmp(argv[i], "--no-sound") == 0) { no_sound = true; no_music = true; }
		else if (strcmp(argv[i], "--help") == 0) { 
			printf(	"\t--help\t\t\tshow this help\n"
					"\t--no-gl\t\t\tdisable GL renderer\n"
					"\t--dx\t\t\tenable directX(tm) renderer (win32 only)\n"
					"\t-2 -3\t\t\tenlarge video mode to 1024x768 or 1280x1024\n"
					"\t--map=xx\t\tload xx as map, start single player\n" 
					"\t--connect=ip/host\tconnect to given host as mp-client\n" 
					"\t--no-sound\t\tdisable sound.\n" 
				  );
			exit(0);
		}
		else throw_ex(("unrecognized option: '%s'", argv[i]));
	}
	
	LOG_DEBUG(("gl: %s, vsync: %s, dx: %s", _opengl?"yes":"no", vsync?"yes":"no", dx?"yes":"no"));
#ifdef WIN32
	_putenv("SDL_VIDEO_RENDERER=gdi");

	if (dx) 
#if SDL_MAJOR_VERSION >= 1 && SDL_MINOR_VERSION >= 3
		_putenv("SDL_VIDEO_RENDERER=d3d");
#else
		_putenv("SDL_VIDEODRIVER=directx");
#endif

#endif

//opengl renderer
#if SDL_MAJOR_VERSION >= 1 && SDL_MINOR_VERSION >= 3
	if (_opengl)
		_putenv("SDL_VIDEO_RENDERER=opengl");
#endif

	LOG_DEBUG(("initializing SDL..."));
#ifdef DEBUG
	sdlx::System::init(SDL_INIT_EVERYTHING | SDL_INIT_NOPARACHUTE);
#else
	sdlx::System::init(SDL_INIT_EVERYTHING);
#endif

	if (_opengl) {
		LOG_DEBUG(("loading GL library"));
		if (SDL_GL_LoadLibrary(NULL) == -1) 
			throw_sdl(("SDL_GL_LoadLibrary"));

	}
	
	int default_flags = sdlx::Surface::Hardware | sdlx::Surface::Alpha | (_opengl? SDL_OPENGL: 0) ;
#ifdef USE_GLSDL
	if (_opengl) {
		default_flags &= ~SDL_OPENGL;
		default_flags |= SDL_GLSDL;
	}
#endif

	sdlx::Surface::setDefaultFlags(default_flags);

	LOG_DEBUG(("initializing SDL_ttf..."));
	sdlx::TTF::init();

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
	
	int flags = SDL_HWSURFACE | SDL_ANYFORMAT;
	//if (doublebuf)
	flags |= SDL_DOUBLEBUF;
	
	if (fullscreen) flags |= SDL_FULLSCREEN;

	LOG_DEBUG(("setting caption..."));		
	SDL_WM_SetCaption(("Battle tanks - " + getVersion()).c_str(), "btanks");
	
	Mixer->init(no_sound, no_music);
		
	Mixer->loadPlaylist(data_dir + "/playlist");
	Mixer->play();
	
	if (_opengl) {
#if SDL_VERSION_ATLEAST(1,2,10)
		LOG_DEBUG(("setting GL swap control to %d...", vsync?1:0));
		int r = SDL_GL_SetAttribute( SDL_GL_SWAP_CONTROL, vsync?1:0);
		if (r == -1) 
			LOG_WARN(("cannot set SDL_GL_SWAP_CONTROL."));
#ifdef WIN32
		if (!vsync) {
			typedef void (APIENTRY * WGLSWAPINTERVALEXT) (int);
			WGLSWAPINTERVALEXT wglSwapIntervalEXT = (WGLSWAPINTERVALEXT) 
			wglGetProcAddress("wglSwapIntervalEXT");
			if (wglSwapIntervalEXT) {
				LOG_DEBUG(("disabling vsync with SwapIntervalEXT(0)..."));
			    wglSwapIntervalEXT(0); // disable vertical synchronisation
			}
		}
#endif

		LOG_DEBUG(("setting GL accelerated visual..."));

		r = SDL_GL_SetAttribute( SDL_GL_ACCELERATED_VISUAL, 1 );
		if (r == -1) 
			LOG_WARN(("cannot set SDL_GL_ACCELERATED_VISUAL."));
#endif
		
		SDL_GL_SetAttribute( SDL_GL_DOUBLEBUFFER, 1 );
		//SDL_GL_SetAttribute( SDL_GL_ALPHA_SIZE, 8 );
	
	
		//_window.setVideoMode(w, h, 0,  SDL_OPENGL | SDL_OPENGLBLIT | flags );
#ifdef USE_GLSDL
		flags |= SDL_GLSDL;
#endif
		_window.setVideoMode(w, h, 0, flags );
	} else {
		_window.setVideoMode(w, h, 0, flags);
	}
	
	LOG_DEBUG(("created main surface. (%dx%dx%d, %s)", w, h, _window.getBPP(), ((_window.getFlags() & SDL_HWSURFACE) == SDL_HWSURFACE)?"hardware":"software"));

	sdlx::System::probeVideoMode();	
#if 0
	{
		SDL_Rect **modes;
		int i;

		/* Get available fullscreen/hardware modes */
		modes = SDL_ListModes(NULL, SDL_FULLSCREEN|SDL_HWSURFACE);

		/* Check is there are any modes available */
		if(modes == (SDL_Rect **)0) 
			throw_ex(("No video modes available"));
    
	    /* Check if our resolution is restricted */
    	if(modes == (SDL_Rect **)-1){
			LOG_DEBUG(("all resolutions available."));
		} else {
			/* Print valid modes */
			LOG_DEBUG(("available modes:"));
			for(i=0;modes[i];++i)
				LOG_DEBUG(("\t%dx%d", modes[i]->w, modes[i]->h));
		}
	}
#endif

	LOG_DEBUG(("initializing menus..."));		
	_main_menu.init(w, h);	

	_paused = false;
	_running = true;

	_window.update();
	
	LOG_DEBUG(("initializing resource manager..."));
	ResourceManager->init("data/resources.xml");
	
	if (_show_fps) {
		LOG_DEBUG(("creating `digits' object..."));
		_fps = ResourceManager->createObject("damage-digits", "damage-digits");
		_fps->onSpawn();
		_fps->speed = 0;
	} else _fps = NULL;

	LOG_DEBUG(("installing callbacks..."));
	key_signal.connect(sigc::mem_fun(this, &IGame::onKey));
	_main_menu.menu_signal.connect(sigc::mem_fun(this, &IGame::onMenu));
	
	if (_preload_map.size()) {
		LOG_DEBUG(("starting predefined map %s...", _preload_map.c_str()));
		loadMap(_preload_map);
		
		//_my_index = spawnPlayer("tank", "green-tank", "keys");
		_my_index = spawnPlayer("launcher", "green-launcher", "keys");
		spawnPlayer("ai-tank", "green-tank", "ai");
		_main_menu.setActive(false);
	}
	if (_autojoin) {
		onMenu("m-join");
		_main_menu.setActive(false);
	}
}

void IGame::onKey(const Uint8 type, const SDL_keysym key) {
	if (key.sym == SDLK_ESCAPE && type == SDL_KEYUP) {
		LOG_DEBUG(("escape hit, paused: %s", _paused?"true":"false"));
		_paused = !_paused;
		_main_menu.setActive(_paused);
	}
}

void IGame::onMenu(const std::string &name) {
	if (name == "quit") 
		_running = false;
	else if (name.substr(0, 6) == "start:") {
		LOG_DEBUG(("start single player requested"));
		clear();
		_main_menu.reset();
		const std::string vehicle = name.substr(6);
		GET_CONFIG_VALUE("stubs.default-map", std::string, map, "survival");
		loadMap(map);
		
		static const char * colors[4] = {"green", "red", "yellow", "cyan"};
		std::string animation = colors[mrt::random(4)];
		animation += "-" + vehicle;

		GET_CONFIG_VALUE("player.control-method", std::string, cm, "keys");		
		_my_index = spawnPlayer(vehicle, animation, cm);
		spawnPlayer("ai-tank", "green-tank", "ai");
		_players[_my_index].viewport = _window.getSize();
		_players[_my_index].visible = true;
	} else if (name == "s-start") {
		LOG_DEBUG(("start split screen game requested"));
		clear();
		_main_menu.reset();
		GET_CONFIG_VALUE("stubs.default-vehicle-1", std::string, vehicle1, "tank");
		GET_CONFIG_VALUE("stubs.default-vehicle-2", std::string, vehicle2, "tank");
		GET_CONFIG_VALUE("stubs.default-map", std::string, map, "survival");
		loadMap(map);

		static const char * colors[4] = {"green", "red", "yellow", "cyan"};
		std::string animation1 = colors[mrt::random(4)];
		std::string animation2 = colors[mrt::random(4)];
		animation1 += "-" + vehicle1;
		animation2 += "-" + vehicle2;

		GET_CONFIG_VALUE("player.control-method-1", std::string, cm, "keys-1");		
		GET_CONFIG_VALUE("player.control-method-2", std::string, cm2, "keys-2");
		
		int p1 = _my_index = spawnPlayer(vehicle1, animation1, cm);
		int p2 = spawnPlayer(vehicle2, animation2, cm2);
		
		v3<int> ts = Map->getTileSize();
		int w = _window.getSize().w / 2;

		_players[p1].viewport = _window.getSize();
		_players[p1].viewport.w = w;
		_players[p1].visible = true;

		_players[p2].viewport = _window.getSize();
		_players[p2].viewport.x = _players[_my_index].viewport.w;
		_players[p2].viewport.w = w;
		_players[p2].visible = true;
		LOG_DEBUG(("p1: %d %d %d %d", _players[p1].viewport.x, _players[p1].viewport.y, _players[p1].viewport.w, _players[p1].viewport.h));
		LOG_DEBUG(("p2: %d %d %d %d", _players[p2].viewport.x, _players[p2].viewport.y, _players[p2].viewport.w, _players[p2].viewport.h));

	} else if (name == "m-start") {
		LOG_DEBUG(("start multiplayer server requested"));
		clear();
		GET_CONFIG_VALUE("stubs.default-mp-map", std::string, map, "country2");
		loadMap(map);
		GET_CONFIG_VALUE("stubs.default-mp-vehicle", std::string, vehicle, "tank");

		GET_CONFIG_VALUE("player.control-method", std::string, cm, "keys");		
		_my_index = spawnPlayer(vehicle, "green-" + vehicle, cm);
		_players[_my_index].viewport = _window.getSize();
		_players[_my_index].visible = true;
		
		_server = new Server;
		_server->init(9876);
	} else if (name == "m-join") {
		clear();
		unsigned port = 9876;
		TRY {
			_client = new Client;
			_client->init(_address, port);
		} CATCH("_client.init", { delete _client; _client = NULL; return; });
		
		_main_menu.setActive(false);
	}
}


void IGame::loadMap(const std::string &name) {
	_main_menu.setActive(false);
	IMap &map = *IMap::get_instance();
	map.load(name);
	
	const v3<int> size = map.getSize();
	_players.clear();

	for (IMap::PropertyMap::iterator i = map.properties.begin(); i != map.properties.end(); ++i) {
		if (i->first.substr(0, 6) != "spawn:" && i->first.substr(0, 7) != "object:") {
			continue;
		}
	
		v3<int> pos;
		std::string pos_str = i->second;
		const bool tiled_pos = pos_str[0] == '@';
		if (tiled_pos) { 
			pos_str = pos_str.substr(1);
		}
		pos.fromString(pos_str);
		if (tiled_pos) {
			v3<int> tile_size = Map->getTileSize();
			pos.x *= tile_size.x;
			pos.y *= tile_size.y;
			//keep z untouched.
		}

		if (pos.x < 0) 
			pos.x += size.x;
		if (pos.y < 0) 
			pos.y += size.y;

		if (i->first.substr(0, 6) == "spawn:") {
			LOG_DEBUG(("spawnpoint: %d,%d", pos.x, pos.y));
			
			PlayerSlot slot;
			slot.position = pos;
			_players.push_back(slot);
		} else {
			std::vector<std::string> res;
			mrt::split(res, i->first, ":");
			if (res.size() > 2 && res[0] == "object") {
				//LOG_DEBUG(("object %s, animation %s, pos: %s", res[1].c_str(), res[2].c_str(), i->second.c_str()));
				Item item;
				Object *o = ResourceManager->createObject(res[1], res[2]);
				World->addObject(o, pos.convert<float>());
				
				item.classname = res[1];
				item.animation = res[2];
				item.position = pos;
				item.dead_on = 0;
				
				
				item.id = o->getID();
				_items.push_back(item);
			}
		}
	}
	LOG_DEBUG(("%u items on map.", (unsigned) _items.size()));
}

void IGame::checkItems() {
	if (_client != NULL) //no need for multiplayer.
		return;
	
	for(Items::iterator i = _items.begin(); i != _items.end(); ++i) {
		Item &item = *i;
		if (World->getObjectByID(item.id) != NULL)
			continue;
		Uint32 ticks = SDL_GetTicks();
		if (item.dead_on == 0) {
			item.dead_on = ticks;
			LOG_DEBUG(("item %d:%s:%s is dead, log dead time.", item.id, item.classname.c_str(), item.animation.c_str()));
			continue;
		}
		int rt;
		Config->get("map." + item.classname + ".respawn-interval", rt, 5); 
		if (rt == 0) 
			continue;
		if (((ticks - item.dead_on) / 1000) >= (unsigned)rt) {
			//respawning item
			LOG_DEBUG(("respawning item: %s:%s", item.classname.c_str(), item.animation.c_str()));
			Object *o = ResourceManager->createObject(item.classname, item.animation);
			World->addObject(o, item.position.convert<float>());
			item.id = o->getID();
			item.dead_on = 0;
		}
	}
}


void IGame::createControlMethod(PlayerSlot &slot, const std::string &control_method) {
	delete slot.control_method;
	slot.control_method = NULL;

	if (control_method == "keys") {
		slot.control_method = new KeyPlayer(SDLK_UP, SDLK_DOWN, SDLK_LEFT, SDLK_RIGHT, SDLK_LCTRL, SDLK_LALT);
	} else if (control_method == "keys-1") {
		slot.control_method = new KeyPlayer(SDLK_r, SDLK_f, SDLK_d, SDLK_g, SDLK_q, SDLK_a);
	} else if (control_method == "keys-2") {
		slot.control_method = new KeyPlayer(SDLK_UP, SDLK_DOWN, SDLK_LEFT, SDLK_RIGHT, SDLK_RCTRL, SDLK_RSHIFT);
	} else if (control_method == "mouse") {
		slot.control_method = new MouseControl();
	} else if (control_method == "network") {
		slot.control_method = new ExternalControl;
		slot.remote = true;
	} else if (control_method != "ai") {
		throw_ex(("unknown control method '%s' used", control_method.c_str()));
	}
}

const int IGame::spawnPlayer(const std::string &classname, const std::string &animation, const std::string &control_method) {
	size_t i, n = _players.size();
	for(i = 0; i < n; ++i) {
		if (_players[i].obj == NULL)
			break;
	}
	if (i == n) 
		throw_ex(("no available slots found from %d", n));
	PlayerSlot &slot = _players[i];

	createControlMethod(slot, control_method);
	
	LOG_DEBUG(("player: %s.%s using control method: %s", classname.c_str(), animation.c_str(), control_method.c_str()));
	spawnPlayer(slot, classname, animation);
	return i;
}

void IGame::spawnPlayer(PlayerSlot &slot, const std::string &classname, const std::string &animation) {
	Object *obj = ResourceManager->createObject(classname, animation);
	assert(obj != NULL);

	World->addObject(obj, slot.position.convert<float>());
	Object *spawn = World->spawn(obj, "spawn-shield", "spawning", v3<float>::empty, v3<float>::empty);
	spawn->follow(obj, Centered);

	slot.obj = obj;
	slot.classname = classname;
	slot.animation = animation;
}



void IGame::run() {
	LOG_DEBUG(("entering main loop"));
	SDL_Event event;
	IMap &map = *IMap::get_instance();

	sdlx::Rect window_size = _window.getSize();
	
	GET_CONFIG_VALUE("engine.fps-limit", int, fps_limit, 1000);
	
	float fr = fps_limit;
	int max_delay = 1000/fps_limit;
	LOG_DEBUG(("fps_limit set to %d, maximum frame delay: %d", fps_limit, max_delay));

	while (_running) {
		Uint32 t_start  = SDL_GetTicks();
#ifdef SHOW_PERFSTATS
		Uint32 t_tick_n = t_start, t_tick_w = t_start, t_tick_s = t_start, t_tick_c = t_start;
#endif
		
		while (SDL_PollEvent(&event)) {
			switch(event.type) {
			case SDL_KEYDOWN:
#ifndef WIN32
				if (event.key.keysym.sym==SDLK_f && event.key.keysym.mod & KMOD_SHIFT) {
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
				if (event.key.keysym.sym==SDLK_d && event.key.keysym.mod & KMOD_SHIFT && _my_index >= 0) {
					_players[_my_index].obj->emit("death", 0);
					break;
				}
			case SDL_KEYUP:
				key_signal.emit(event.key.type, event.key.keysym);
			break;
			case SDL_MOUSEBUTTONDOWN:
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
		
		
		if (_running && !_paused) {
			updatePlayers();
			if (_check_items.tick(dt)) {
				checkItems();
			}
#ifdef SHOW_PERFSTATS
			t_tick_n = SDL_GetTicks();
#endif
		
			World->tick(dt);
			Mixer->updateObjects();
#ifdef SHOW_PERFSTATS
			t_tick_w = SDL_GetTicks();
#endif
			
			if (_server) {
				if (_next_sync.tick(dt) && _server->active()) {
					Message m(Message::UpdateWorld);
					{
						mrt::Serializator s;
						World->generateUpdate(s);
						m.data = s.getData();
					}
					LOG_DEBUG(("sending world update... (size: %u)", m.data.getSize()));
					_server->broadcast(m);
				}
				_server->tick(dt);
			}
#ifdef SHOW_PERFSTATS
			t_tick_s = SDL_GetTicks();
#endif

			if (_client) {
				_client->tick(dt);
				if (_ping && t_start >= _next_ping) {
					ping();
					GET_CONFIG_VALUE("multiplayer.ping-interval", int, ping_interval, 1500);
					_next_ping = t_start + ping_interval; //fixme: hardcoded value
				}
			}

#ifdef SHOW_PERFSTATS
			t_tick_c = SDL_GetTicks();
#endif				
			for(unsigned int pi = 0; pi < _players.size(); ++pi) {
				PlayerSlot &slot = _players[pi];
				const Object * p = slot.obj;
				if (p == NULL || !slot.visible)
					continue; 
					
				v3<float> pos, vel;
				p->getInfo(pos, vel);

				if ((int)pi == _my_index)
					Mixer->setListener(pos, vel);
					
				sdlx::Rect passive_viewport;
				passive_viewport.w = passive_viewport.x = slot.viewport.w / 3;
				passive_viewport.h = passive_viewport.y = slot.viewport.h / 3;
				sdlx::Rect passive_viewport_stopzone(passive_viewport);
	
				{
					int xmargin = passive_viewport_stopzone.w / 4;
					int ymargin = passive_viewport_stopzone.h / 4;
					passive_viewport_stopzone.x += xmargin;
					passive_viewport_stopzone.y += ymargin;
					passive_viewport_stopzone.w -= 2*xmargin;
					passive_viewport_stopzone.h -= 2*ymargin;
				}


				//LOG_DEBUG(("player[0] %f, %f", vel.x, vel.y));
				int wx = (int)(pos.x - slot.mapx);
				int wy = (int)(pos.y - slot.mapy);
				if (passive_viewport_stopzone.in(wx, wy)) {
					slot.mapvx = 0; 
					slot.mapvy = 0;
				} else {
					slot.mapvx = p->speed * 2 * (wx - passive_viewport.x) / passive_viewport.w ;
					slot.mapvy = p->speed * 2 * (wy - passive_viewport.y) / passive_viewport.h ;
					/*
					LOG_DEBUG(("position : %f %f viewport: %d %d(passive:%d %d %d %d) mapv: %f %f", x, y,
						viewport.x, viewport.y, passive_viewport.x, passive_viewport.y, passive_viewport.w, passive_viewport.h, 
						mapvx, mapvy));
					*/
				}
			}
		}
#ifdef SHOW_PERFSTATS
		Uint32 t_tick = SDL_GetTicks();
#endif

		_window.fillRect(window_size, 0);
	
		for(unsigned p = 0; p < _players.size(); ++p) {
			PlayerSlot &slot = _players[p];
			if (!slot.visible)
				continue;
				
			if (_shake > 0) {
				slot.viewport.y += _shake_int;
			}		
	
			World->render(_window, sdlx::Rect((int)slot.mapx, (int)slot.mapy, slot.viewport.w, slot.viewport.h),  slot.viewport);
	
			if (_shake >0) {
				slot.viewport.y -= _shake_int;
				_shake_int = -_shake_int;
				_shake -= dt;
			}

		}
		_main_menu.render(_window);
		

		if (_show_fps) {
			_fps->hp = (int)fr;
			_fps->render(_window, 0, 0);
		}		
		
		if (map.loaded()) {
			const v3<int> world_size = map.getSize();
			for(unsigned p = 0; p < _players.size(); ++p) {
				PlayerSlot &slot = _players[p];
				slot.mapx += slot.mapvx * dt;
				slot.mapy += slot.mapvy * dt;
			
				if (slot.mapx < 0) 
					slot.mapx = 0;
				if (slot.mapx + slot.viewport.w > world_size.x) 
					slot.mapx = world_size.x - slot.viewport.w;

				if (slot.mapy < 0) 
					slot.mapy = 0;
				if (slot.mapy + slot.viewport.h > world_size.y) 
					slot.mapy = world_size.y - slot.viewport.h;
			
				//LOG_DEBUG(("%f %f", mapx, mapy));
			}
		}

#ifdef SHOW_PERFSTATS
		Uint32 t_render = SDL_GetTicks();
#endif

		_window.flip();
		if (_opengl) {
			//glFlush_ptr.call();
		}


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
	LOG_DEBUG(("shutting down, freeing surface"));
	delete _fps;
	_fps = NULL;
	
	_running = false;
	_window.free();
	
	_main_menu.deinit();
}

const int IGame::onConnect(Message &message) {
	const std::string an = "red-tank";
	LOG_DEBUG(("new client! spawning player:%s", an.c_str()));
	const int client_id = spawnPlayer("tank", an, "network");
	LOG_DEBUG(("client #%d", client_id));

	LOG_DEBUG(("sending server status message..."));
	message.type = Message::ServerStatus;
	message.set("map", Map->getName());
	message.set("version", getVersion());

	mrt::Serializator s;
	World->serialize(s);
	s.add(_players[client_id].obj->getID());

	message.data = s.getData();
	LOG_DEBUG(("world: %s", message.data.dump().c_str()));
	return client_id;
}

void IGame::onDisconnect(const int id) {
	if ((unsigned)id >= _players.size()) {
		LOG_ERROR(("player %d doesnt exists, so cannot disconnect.", id));
		return;
	}
	PlayerSlot &slot = _players[id];
	if (slot.obj)
		slot.obj->emit("death", NULL);
	
	slot.clear();
}


void IGame::onMessage(const int id, const Message &message) {
TRY {
	LOG_DEBUG(("incoming message %s", message.getType()));
	switch(message.type) {
	case Message::ServerStatus: {
		LOG_DEBUG(("server version: %s", message.get("version").c_str()));
		LOG_DEBUG(("loading map..."));
		Map->load(message.get("map"));
		
		mrt::Serializator s(&message.data);
		World->deserialize(s);
		
		int my_id;
		s.get(my_id);
		LOG_DEBUG(("my_id = %d", my_id));
		_players.clear();
		_my_index = 0;
		
		Object * player = World->getObjectByID(my_id);
		if (player == NULL) 
			throw_ex(("invalid object id returned from server. (%d)", my_id));
		_players.push_back(player);
		assert(!_players.empty());
		PlayerSlot &slot = _players[_players.size() - 1];
		slot.classname = slot.obj->registered_name;
		slot.animation = slot.obj->animation;
		slot.viewport = _window.getSize();
		slot.visible = true;
		
		assert(slot.control_method == NULL);
		GET_CONFIG_VALUE("player.control-method", std::string, control_method, "keys");	
		createControlMethod(slot, control_method);

		LOG_DEBUG(("players = %d", _players.size()));
		_next_ping = 0;
		_ping = true;
		break;	
	}
	case Message::UpdateWorld: {
		mrt::Serializator s(&message.data);
		World->applyUpdate(s, _trip_time / 1000.0);
		break;
	} 
	case Message::PlayerState: {
		mrt::Serializator s(&message.data);
		if (id < 0 || (unsigned)id >= _players.size())
			throw_ex(("player id exceeds players count (%d/%d)", id, _players.size()));
		PlayerSlot &slot = _players[id];
		ExternalControl * ex = dynamic_cast<ExternalControl *>(slot.control_method);
		if (ex == NULL)
			throw_ex(("player with id %d uses non-external control method", id));
		ex->state.deserialize(s);
		//World->tick(*slot.obj, slot.trip_time / 1000.0);
		break;
	} 
	case Message::UpdatePlayers: { 
		mrt::Serializator s(&message.data);
		while(!s.end()) {
			int id;
			s.get(id);
			if (id == _players[_my_index].obj->getID())
				throw_ex(("server sent update for your state, bug."));
			PlayerState state; 
			state.deserialize(s);
			Object *o = World->getObjectByID(id);
			if (o != NULL) {
				World->tick(*o, -_trip_time / 1000.0); //back in time ;)
				o->updatePlayerState(state);
				World->tick(*o, _trip_time / 1000.0);
			} else {
				LOG_WARN(("skipped state update for object id %d", id));
			}
		}
		break;
	} 
	case Message::Ping: {
		Message m(Message::Pang);
		m.data = message.data;
		size_t size = m.data.getSize();
		m.data.reserve(size + sizeof(unsigned int));
		
		unsigned int ts = SDL_GetTicks();
		*(unsigned int *)((unsigned char *)m.data.getPtr() + size) = ts;
		_server->send(id, m);
		break;
	}
	
	case Message::Pang: {
		const mrt::Chunk &data = message.data;
		float ping = extractPing(data);
		GET_CONFIG_VALUE("multiplayer.ping-interpolation-multiplier", int, pw, 3);
		_trip_time = (pw * ping + _trip_time) / (pw + 1);
		
		GET_CONFIG_VALUE("multiplayer.ping-interval", int, ping_interval, 1500);

		_next_ping = SDL_GetTicks() + ping_interval; 
		
		LOG_DEBUG(("ping = %g", _trip_time));
		Message m(Message::Pong);
		m.data.setData((unsigned char *)data.getPtr() + sizeof(unsigned int), data.getSize() - sizeof(unsigned int));
		_client->send(m);
		break;
	}
	
	case Message::Pong: {
		if (id < 0 || (unsigned)id >= _players.size())
			throw_ex(("player id exceeds players count (%d/%d)", id, _players.size()));
		float ping = extractPing(message.data);
		
		_players[id].trip_time = (3 * ping + _players[id].trip_time) / 4;
		LOG_DEBUG(("player %d: ping: %g ms", id, ping));		
		break;
	}
	
	case Message::Respawn: {
		TRY {
		if (id < 0 || (unsigned)id >= _players.size())
			throw_ex(("player id exceeds players count (%d/%d)", id, _players.size()));
		PlayerSlot &slot = _players[_my_index];
		mrt::Serializator s(&message.data);
		World->applyUpdate(s, _trip_time / 1000.0);
		int id;
		s.get(id);
		slot.obj = World->getObjectByID(id);
		} CATCH("message::respawn", throw;);
	break;
	}
	default:
		LOG_WARN(("unhandled message: %s\n%s", message.getType(), message.data.dump().c_str()));
	};
} CATCH("onMessage", { 
	if (_server) 
		_server->disconnect(id);
	if (_client) 
		_client->disconnect();
});
}

const float IGame::extractPing(const mrt::Chunk &data) const {
	if (data.getSize() < sizeof(unsigned int))
		throw_ex(("invalid pong recv'ed. (size: %u)", data.getSize()));
	
	unsigned int ts = * (unsigned int *)data.getPtr();
	Uint32 ticks = SDL_GetTicks();
	float delta = (int)(ticks - ts);
	if (delta < 0) delta = -delta; //wrapped around.
	if (delta > 10000)
		throw_ex(("server returns bogus timestamp value. [%g]", delta));
	delta /= 2;
	return delta;
}


void IGame::clear() {
	Mixer->cancelAll();

	GET_CONFIG_VALUE("multiplayer.sync-interval", float, sync_interval, 103.0/101);
	_next_sync.set(sync_interval);

	LOG_DEBUG(("deleting server/client if exists."));
	_ping = false;
	delete _server; _server = NULL;
	delete _client; _client = NULL;

	LOG_DEBUG(("cleaning up players..."));
	_players.clear();
	_my_index = -1;
	LOG_DEBUG(("cleaning up world"));
	_items.clear();
	World->clear();
	_paused = false;
	Map->clear();
}

void IGame::PlayerSlot::clear() {
	obj = NULL;
	if (control_method != NULL) {
		delete control_method; 
		control_method = NULL;
	}
	animation.clear();
	classname.clear();
	remote = false;
}

IGame::PlayerSlot::~PlayerSlot() {
	clear();
}

void IGame::updatePlayers() {
	int n = _players.size();
	for(int i = 0; i < n; ++i) {
		PlayerSlot &slot = _players[i];
		if (slot.obj == NULL || World->exists(slot.obj)) 
			continue;
		LOG_DEBUG(("player in slot %d is dead. respawning.", i));
		spawnPlayer(slot, slot.classname, slot.animation);
		if (slot.remote) {
			Message m(Message::Respawn);
			mrt::Serializator s;
			World->generateUpdate(s);
			s.add(slot.obj->getID());
			m.data = s.getData();
			_server->send(i, m);
		}
	}
	
	bool updated = false;
	
	for(int i = 0; i < n; ++i) {
		PlayerSlot &slot = _players[i];
		if (slot.control_method != NULL) {
			assert(slot.obj != NULL);
			PlayerState old_state = slot.obj->getPlayerState();
			PlayerState state = old_state;
			slot.control_method->updateState(state);
			if (slot.obj->updatePlayerState(state)) {
				LOG_DEBUG(("player[%d] updated state: %s -> %s", i, old_state.dump().c_str(), state.dump().c_str()));
				updated = true;
				slot.state = state;
				slot.need_sync = true;
			}
			if (slot.need_sync && slot.remote) {
				LOG_DEBUG(("correcting remote player. "));
				slot.obj->getPlayerState() = old_state;
				World->tick(*slot.obj, -slot.trip_time / 1000.0);
				slot.obj->getPlayerState() = state;
				World->tick(*slot.obj, slot.trip_time / 1000.0);
			}
		}
	}
				
	if (_client && _my_index >= 0 && _players[_my_index].need_sync)	{
		_client->notify(_players[_my_index].state);
		_players[_my_index].need_sync = false;
	}
	//cross-players state exchange
	if (_server && updated) {
		for(int i = 0; i < n; ++i) {
			if (i == _my_index || _players[i].obj == NULL) continue;
			
			bool send = false;
			mrt::Serializator s;
			for(int j = 0; j < n; ++j) {
				if (i == j) 
					continue;

				PlayerSlot &slot = _players[j];
				if (slot.need_sync) {
					//LOG_DEBUG(("object in slot %d: %s (%d) need sync", j, slot.obj->registered_name.c_str(), slot.obj->getID()));
					s.add(slot.obj->getID());
					slot.state.serialize(s);
					send = true;
				}
			}
			if (send) {
				Message m(Message::UpdatePlayers);
				m.data = s.getData();
				_server->send(i, m);
			}
		}
		for(int i = 0; i < n; ++i) {
			_players[i].need_sync = false;
		}
	}
}

void IGame::shake(const float duration, const int intensity) {
	_shake = duration;
	_shake_int = intensity;
}

void IGame::ping() {
	Message m(Message::Ping);
	unsigned int ts = SDL_GetTicks();
	LOG_DEBUG(("ping timestamp = %u", ts));
	m.data.setData(&ts, sizeof(ts));
	_client->send(m);
}

const int IGame::getMyPlayerIndex() const {
	return _my_index;
}

IGame::PlayerSlot &IGame::getPlayerSlot(const int idx) {
	return _players[idx];
}

void IGame::screen2world(v3<float> &pos, const int p, const int x, const int y) {
	PlayerSlot &slot = _players[p];
	pos.x = slot.mapx + x;
	pos.y = slot.mapx + y;
}

