#include "sdlx/fps.h"
#include "sdlx/sdl_ex.h"
#include "SDL/SDL.h"

using namespace sdlx;


//gfx version

FPSManager::FPSManager() {	
	SDL_initFramerate(&_fps);
}

void FPSManager::set(const int rate) {
	if (SDL_setFramerate(&_fps, rate))
		throw_sdl(("SDL_initFramerate"));
}

const int FPSManager::get() {
	int r = SDL_getFramerate(&_fps);
	if (r == -1) 
		throw_sdl(("SDL_getFramerate"));
	return r;
}

void FPSManager::delay() {
	SDL_framerateDelay(&_fps);
}

/*

FPSManager::FPSManager() {	
	_last_tick = SDL_GetTicks();
	_rate = 50;
	_last_fps = _rate / 2;
}

void FPSManager::set(const int rate) {
	if (rate < 1 || rate > 200) 
		throw_ex(("fps %d is not allowed", rate));
	_rate = rate;
}

const int FPSManager::get() {
	unsigned t = SDL_GetTicks();
	unsigned r = 1000/(t - _last_tick);
	r = (r + _last_fps)/2;
	return _last_fps = r;
}

void FPSManager::delay() {
	unsigned d = 1000 / _rate;
	unsigned t = SDL_GetTicks();
	unsigned l = t - _last_tick;
	_last_tick = t;
	
	if (l < d) 
		SDL_Delay(d - l);
}
*/
