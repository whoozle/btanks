#include "net_ex.h"
#include <SDL/SDL_net.h>

using namespace sdlx;

NetException::NetException() {}

const std::string NetException::getCustomMessage() {
	return SDLNet_GetError();
}

NetException::~NetException() throw() {}

