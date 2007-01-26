#ifndef __BTANKS_CHEATER_H__
#define __BTANKS_CHEATER_H__

#include <string>
#include <vector>
#include <deque>

#include "sdlx/sdlx.h"
#include <sigc++/sigc++.h>

class Cheater : public sigc::trackable {
public: 
	Cheater();
private: 
	bool onKey(const SDL_keysym sym);
	std::vector<std::string> _cheats;
	size_t _buf_size;
	char _buf[16];
};

#endif

