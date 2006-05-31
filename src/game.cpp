#include "game.h"
#include "version.h"

#include "mrt/logger.h"
#include "mrt/exception.h"

#include "sdlx/system.h"
#include "sdlx/sdl_ex.h"
#include "sdlx/joystick.h"
#include "sdlx/ttf.h"
#include "sdlx/color.h"
#include "sdlx/fps.h"

#include "SDL/SDL_gfxPrimitives.h"

IMPLEMENT_SINGLETON(Game, IGame)

IGame::IGame() {
	LOG_DEBUG(("IGame ctor"));
}
IGame::~IGame() {}

const std::string IGame::data_dir = "data";

void IGame::init(const int argv, const char **argc) {

#ifdef __linux__
//	putenv("SDL_VIDEODRIVER=directfb");
#endif

	LOG_DEBUG(("initializing SDL..."));
	sdlx::System::init(SDL_INIT_EVERYTHING);
	LOG_DEBUG(("initialzing SDL_ttf..."));
	sdlx::TTF::init();
	
	LOG_DEBUG(("probing for joysticks"));
	int jc = sdlx::Joystick::getCount();
	if (jc > 0) {
		LOG_DEBUG(("found %d joystick(s)", jc));
		sdlx::Joystick::sendEvents(true);
		
		for(int i = 0; i < jc; ++i) {
			LOG_DEBUG(("%d: %s", i, sdlx::Joystick::getName(i).c_str()));
			sdlx::Joystick j;
			j.open(i);
			
			j.close();
		}
	}
	LOG_DEBUG(("probing video info..."));
	char drv_name[256];
	if (SDL_VideoDriverName(drv_name, sizeof(drv_name)) == NULL)
		throw_sdl(("SDL_VideoDriverName"));
	LOG_DEBUG(("driver name: %s", drv_name));
	const SDL_VideoInfo * vinfo = SDL_GetVideoInfo();
	if (vinfo == NULL)
		throw_sdl(("SDL_GetVideoInfo()"));
	LOG_DEBUG(("hw_available: %u; wm_available: %u;\n\tblit_hw: %u; blit_hw_CC:%u; blit_hw_A:%u; blit_sw:%u; blit_sw_CC:%u; blit_sw_A: %u; \n\tblit_fill: %u; video_mem: %u", 
		vinfo->hw_available, vinfo->wm_available, vinfo->blit_hw, vinfo->blit_hw_CC, vinfo->blit_hw_A, vinfo->blit_sw, vinfo->blit_sw_CC, vinfo->blit_sw_A, vinfo->blit_fill, vinfo->video_mem ));
	
	int w = 640, h = 480;
	LOG_DEBUG(("creating main surface. (%dx%d)", w, h));
	_window.setVideoMode(w, h, 24, SDL_ASYNCBLIT | SDL_HWSURFACE | SDL_DOUBLEBUF);
	SDL_WM_SetCaption("Battle tanks - " VERSION_STRING, "btanks");

	_main_menu.init(w, h);	

	_window.update();
	_running = true;
	
	LOG_DEBUG(("installing callbacks..."));
	key_signal.connect(sigc::mem_fun(this, &IGame::onKey));
	_main_menu.menu_signal.connect(sigc::mem_fun(this, &IGame::onMenu));
}

void IGame::onKey(const Uint8 type, const SDL_keysym key) {
/*	if (key.sym == SDLK_ESCAPE && _main_menu.isActive() == false) {
		//pause game
		//...
		LOG_DEBUG(("pause"));
	}
*/}

void IGame::onMenu(const std::string &name) {
	if (name == "quit") 
		_running = false;
	else if (name == "start") {
		LOG_DEBUG(("start single player requested"));
		_main_menu.setActive(false);
		
		_map.load("factory");
	}
}



void IGame::run() {
	SDL_Event event;

	sdlx::Rect window_size = _window.getSize();
	Uint32 black = _window.mapRGB(0, 0, 0);

	float mapx = 0, mapy = 0, mapvx = 100, mapvy = 75;
	int fps_limit = 75;
	
	float fr = fps_limit / 2;
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
			case SDL_KEYUP:
				key_signal.emit(event.key.type, event.key.keysym);
			break;
		    case SDL_QUIT:
				_running = false;
			break;
    		}
		}
		_window.fillRect(window_size, black);
		_map.render(_window, (long)mapx, (long)mapy);
		_main_menu.render(_window);
		
		
		std::string f = mrt::formatString("%d", (int)fr);
		stringRGBA(_window.getSDLSurface(), 3, 3, f.c_str(), 255, 255, 255, 255);
		
		if (_map.loaded()) {
			mapx += mapvx / fr ;
			mapy += mapvy / fr ;
			//LOG_DEBUG(("%f %f", mapx, mapy));
		}
		_window.flip();

		int tdelta = SDL_GetTicks() - tstart;
		fr = (tdelta != 0)? (1000.0 / tdelta): 9999;
		if (tdelta < max_delay)
			SDL_Delay(max_delay - tdelta);
	}

	if (_running)
		throw_sdl(("SDL_WaitEvent"));
}

void IGame::deinit() {
	LOG_DEBUG(("shutting down, freeing surface"));
	_running = false;
	_window.free();
}
