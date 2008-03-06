#include "sdl_ex.h"
#include <SDL.h>

using namespace sdlx;

Exception::Exception() {}

const std::string Exception::getCustomMessage() {
	return SDL_GetError();
}

Exception::~Exception() throw() {}
