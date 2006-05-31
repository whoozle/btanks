#include "sdlx/ttf.h"
#include <stdlib.h>
#include "sdlx/sdl_ex.h"
#include "sdlx/surface.h"


using namespace sdlx;

void TTF::init() {
	TTF_Init();
	atexit(TTF_Quit);
}

TTF::TTF() : _font(NULL) {}
TTF::~TTF() { close(); }
	
void TTF::open(const std::string &fname, const int psize) {
	_font = TTF_OpenFont(fname.c_str(), psize);
	if (_font == NULL)
		throw_sdl(("TTF_OpenFont"));
}

void TTF::renderBlended(sdlx::Surface &result, const std::string &text, const SDL_Color &fg) {
	SDL_Surface *r = TTF_RenderUTF8_Blended(_font, text.c_str(), fg);
	if (r == NULL)
		throw_sdl(("TTF_RenderUTF8_Blended"));
	result.assign(r);
}

void TTF::close() {
	if (_font == NULL) 
		return;
	
	TTF_CloseFont(_font);
	_font = NULL;
}
