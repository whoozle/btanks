#ifndef __BTANKS_CHEATER_H__
#define __BTANKS_CHEATER_H__

#include <string>
#include <vector>
#include <deque>

#include "sdlx/sdlx.h"

class Cheater {
public: 
	Cheater();
private: 
	void onKey(const Uint8 type, const SDL_keysym sym);
	std::vector<std::string> _cheats;
	size_t _buf_size;
	char _buf[16];
};

#endif

