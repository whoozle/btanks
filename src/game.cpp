#include "game.h"
#include "version.h"
#include "world.h"
#include "resource_manager.h"

#include "mrt/logger.h"
#include "mrt/exception.h"

#include "sdlx/system.h"
#include "sdlx/sdl_ex.h"
#include "sdlx/joystick.h"
#include "sdlx/ttf.h"
#include "sdlx/color.h"
#include "sdlx/fps.h"
#include "sdlx/net_ex.h"
#include "sdlx/tcp_socket.h"

#include "net/server.h"
#include "net/client.h"
#include "net/protocol.h"
#include "net/connection.h"

#include "objects/joyplayer.h"
#include "objects/keyplayer.h"
#include "objects/aiplayer.h"

#include <SDL/SDL_gfxPrimitives.h>
#include <SDL/SDL_opengl.h>
#include <SDL/SDL_net.h>

IMPLEMENT_SINGLETON(Game, IGame)

IGame::IGame() {
	LOG_DEBUG(("IGame ctor"));
}
IGame::~IGame() {}

const std::string IGame::data_dir = "data";

typedef void (*glEnable_Func)(GLenum cap);
typedef void (*glBlendFunc_Func) (GLenum sfactor, GLenum dfactor );

template <typename FuncPtr> union SharedPointer {
	FuncPtr call;
	void *ptr;
};

static SharedPointer<glEnable_Func> glEnable_ptr;
static SharedPointer<glBlendFunc_Func> glBlendFunc_ptr;

void IGame::init(const int argv, const char **argc) {

	_server = NULL; _client = NULL;
#ifdef __linux__
//	putenv("SDL_VIDEODRIVER=dga");
#endif
	bool opengl = (argv > 1 && std::string(argc[1]) == "--gl");

	LOG_DEBUG(("initializing SDL..."));
	sdlx::System::init(SDL_INIT_EVERYTHING);

	if (opengl) {
		LOG_DEBUG(("loading GL library"));
		if (SDL_GL_LoadLibrary(NULL) == -1) 
			throw_sdl(("SDL_GL_LoadLibrary"));

		glEnable_ptr.ptr = SDL_GL_GetProcAddress("glEnable");
		if (!glEnable_ptr.ptr)
			throw_ex(("cannot get address of glEnable"));
	
		glBlendFunc_ptr.ptr = SDL_GL_GetProcAddress("glBlendFunc");
		if (!glBlendFunc_ptr.ptr)
			throw_ex(("cannot get address of glBlendFunc"));
	}
	
	sdlx::Surface::setDefaultFlags(sdlx::Surface::Hardware | sdlx::Surface::Alpha | sdlx::Surface::ColorKey);

	LOG_DEBUG(("initializing SDL_ttf..."));
	sdlx::TTF::init();

	LOG_DEBUG(("initializing SDL_net..."));
	if (SDLNet_Init() == -1) {
		throw_net(("SDLNet_Init"));
	}
	
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
	int w = 800, h = 600;
	if (opengl) {
		SDL_GL_SetAttribute( SDL_GL_DOUBLEBUFFER, 1 );
		SDL_GL_SetAttribute( SDL_GL_ALPHA_SIZE, 8 );
	
		glBlendFunc_ptr.call( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA ) ;
		glEnable_ptr.call( GL_BLEND ) ;
	
		_window.setVideoMode(w, h, 32, SDL_OPENGL | SDL_OPENGLBLIT);
	} else {
		_window.setVideoMode(w, h, 32, SDL_ASYNCBLIT | SDL_HWSURFACE | SDL_DOUBLEBUF | SDL_SRCALPHA);
	}
	
	LOG_DEBUG(("created main surface. (%dx%dx%d)", w, h, _window.getBPP()));

	sdlx::System::probeVideoMode();	

	SDL_WM_SetCaption(("Battle tanks - " + getVersion()).c_str(), "btanks");

	_main_menu.init(w, h);	

	_paused = false;
	_running = true;

	_window.update();
	
	LOG_DEBUG(("initializing resource manager..."));
	ResourceManager->init("data/resources.xml");

	LOG_DEBUG(("installing callbacks..."));
	key_signal.connect(sigc::mem_fun(this, &IGame::onKey));
	_main_menu.menu_signal.connect(sigc::mem_fun(this, &IGame::onMenu));
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
	else if (name == "start") {
		LOG_DEBUG(("start single player requested"));
		_main_menu.setActive(false);
		
		_map.load("country");
		Object *player = ResourceManager->createObject("key-player", "green-tank");
		//Object *player = new KeyPlayer("green-tank", SDLK_UP, SDLK_DOWN, SDLK_LEFT, SDLK_RIGHT, SDLK_SPACE);
		//Object *player = new JoyPlayer("green-tank", 0, 0);
		//Object *player = new AIPlayer("green-tank");
		_players.clear();
		_players.push_back(player);
		World->addObject(player, v3<float>(100, 100, 0));

		Player *p = new AIPlayer("red-tank");
		p->setDirection(4);
		sdlx::Rect r = _map.getSize();
		World->addObject(p, v3<float>(r.w - 100, 100, 0));
	} else if (name == "m-start") {
		
		_main_menu.setActive(false);

		_map.load("country");
		Object *player = new KeyPlayer("green-tank", SDLK_UP, SDLK_DOWN, SDLK_LEFT, SDLK_RIGHT, SDLK_SPACE);
		_players.clear();
		_players.push_back(player);
		World->addObject(player, v3<float>(100, 100, 0));

		_server = new Server;
		_server->init(9876);
	} else if (name == "m-join") {
		std::string host = "localhost";
		unsigned port = 9876;
		TRY {
			_client = new Client;
			_client->init(host, port);
		} CATCH("_client.init", { delete _client; _client = NULL; return; });
		
		_main_menu.setActive(false);
	}
}



void IGame::run() {
	SDL_Event event;

	sdlx::Rect window_size = _window.getSize();
	sdlx::Rect viewport = _window.getSize();
	sdlx::Rect passive_viewport;
	passive_viewport.w = passive_viewport.x = viewport.w / 3;
	passive_viewport.h = passive_viewport.y = viewport.h / 3;
	sdlx::Rect passive_viewport_stopzone(passive_viewport);
	
	{
		int xmargin = passive_viewport_stopzone.w / 4;
		int ymargin = passive_viewport_stopzone.h / 4;
		passive_viewport_stopzone.x += xmargin;
		passive_viewport_stopzone.y += ymargin;
		passive_viewport_stopzone.w -= 2*xmargin;
		passive_viewport_stopzone.h -= 2*ymargin;
	}
	
	Uint32 black = _window.mapRGB(0, 0, 0);

	float mapx = 0, mapy = 0, mapvx = 0, mapvy = 0;
	int fps_limit = 50;
	
	float fr = fps_limit;
	int max_delay = 1000/fps_limit;
	
	while (_running) {
		Uint32 tstart = SDL_GetTicks();
		
		while (SDL_PollEvent(&event)) {
			switch(event.type) {
			case SDL_KEYDOWN:
				if (event.key.keysym.sym==SDLK_f && event.key.keysym.mod & KMOD_CTRL) {
					_window.toggleFullscreen();
					break;
				}
				if (event.key.keysym.sym==SDLK_s && event.key.keysym.mod & KMOD_CTRL) {
					_window.saveBMP("screenshot.bmp");
					break;
				}
			case SDL_KEYUP:
				key_signal.emit(event.key.type, event.key.keysym);
			break;
		    case SDL_QUIT:
				_running = false;
			break;
    		}
		}
		
		const float dt = 1.0/fr;
		
		
		if (_running && !_paused) {
			World->tick(_map, dt);
			if (_server) 
				_server->tick(dt);

			if (_client) 
				_client->tick(dt);
			
			if (_players.size()) {
				Object * p = _players[0];
				v3<float> pos, vel;
				if (World->getInfo(p, pos, vel)) {
					//LOG_DEBUG(("player[0] %f, %f, %f", vx, vy, vz));
					int wx = (int)pos.x - viewport.x;
					int wy = (int)pos.y - viewport.y;
					if (passive_viewport_stopzone.in(wx, wy)) {
						mapvx = 0; 
						mapvy = 0;
					} else {
						mapvx = p->speed * 2 * (wx - passive_viewport.x) / passive_viewport.w ;
						mapvy = p->speed * 2 * (wy - passive_viewport.y) / passive_viewport.h ;
						/*
						LOG_DEBUG(("position : %f %f viewport: %d %d(passive:%d %d %d %d) mapv: %f %f", x, y,
							viewport.x, viewport.y, passive_viewport.x, passive_viewport.y, passive_viewport.w, passive_viewport.h, 
							mapvx, mapvy));
						*/
					}
				}
			}
		}
		
		_window.fillRect(window_size, black);
		_map.render(_window, viewport, -1000, 0);
		World->render(_window, viewport);
		_map.render(_window, viewport, 0, 1001);

		_main_menu.render(_window);
		
		
		std::string f = mrt::formatString("%d", (int)fr);
		stringRGBA(_window.getSDLSurface(), 4, 4, f.c_str(), 0,0,0, 255);
		stringRGBA(_window.getSDLSurface(), 3, 3, f.c_str(), 255, 255, 255, 255);
		
		if (_map.loaded()) {
			sdlx::Rect world_size = _map.getSize();
		
			mapx += mapvx * dt;
			mapy += mapvy * dt;
			
			if (mapx < 0) 
				mapx = 0;
			if (mapx + viewport.w > world_size.w) 
				mapx = world_size.w - viewport.w;

			if (mapy < 0) 
				mapy = 0;
			if (mapy + viewport.h > world_size.h) 
				mapy = world_size.h - viewport.h;
			
			viewport.x = (Sint16) mapx;
			viewport.y = (Sint16) mapy;
			
			//LOG_DEBUG(("%f %f", mapx, mapy));
		}
		_window.update();
		_window.flip();
	
		int tdelta = SDL_GetTicks() - tstart;

		if (tdelta < max_delay) {
			//LOG_DEBUG(("tdelta: %d, delay: %d", tdelta, max_delay - tdelta));
			SDL_Delay(max_delay - tdelta);
		}

		tdelta = SDL_GetTicks() - tstart;
		fr = (tdelta != 0)? (1000.0 / tdelta): 1000;
	}

	if (_running)
		throw_sdl(("SDL_WaitEvent"));
}

void IGame::deinit() {
	delete _server; _server = NULL;
	delete _client; _client = NULL;
	LOG_DEBUG(("shutting down, freeing surface"));
	_running = false;
	_window.free();
}

void IGame::notify(const PlayerState& state) {
	if (_client)
		_client->notify(state);
	if (_server) {
		mrt::Serializator s;
		World->serialize(s);

		Message message(UpdateWorld);
		message.data = s.getData();
		_server->broadcast(message);
	}
}

void IGame::onClient(Message &message) {
	LOG_DEBUG(("sending server status message..."));
	message.type = ServerStatus;
	message.set("map", _map.getName());
	message.set("version", getVersion());

	mrt::Serializator s;
	World->serialize(s);

	message.data = s.getData();
	LOG_DEBUG(("world: %s", message.data.dump().c_str()));
}

void IGame::onMessage(const Connection &conn, const Message &message) {
	LOG_DEBUG(("incoming message %d", message.type));
	if (message.type == ServerStatus) {
		LOG_DEBUG(("loading map..."));
		_map.load(message.get("map"));
		
		mrt::Serializator s(&message.data);
		World->deserialize(s);
	} else if (message.type == UpdateWorld) {
		mrt::Serializator s(&message.data);
		World->deserialize(s);
	}
}

