#include "cheater.h"
#include "game.h"
#include <string.h>

Cheater::Cheater() : _buf_size(0) {
	Game->key_signal.connect(sigc::mem_fun(this, &Cheater::onKey));
	_cheats.push_back("skotobaza");
	
	//scan cheats.
	size_t max = 0;
	for(size_t i = 0; i < _cheats.size(); ++i) {
		size_t l = _cheats[i].size();
		if (l > max) 
			max = l;
	}
	assert(max <= sizeof(_buf));
}

#include "world.h"
#include "object.h"

bool Cheater::onKey(const SDL_keysym sym) {
	size_t n = sizeof(_buf)/sizeof(_buf[0]);
	
	if (_buf_size < n - 1) {
		_buf[_buf_size++] = sym.sym;
	} else {
		memmove(_buf, _buf + 1, sizeof(_buf) - sizeof(_buf[0]));
		_buf[_buf_size] = sym.sym;
	}
	
	//LOG_DEBUG(("buf: %s, size: %d of %d", std::string((const char *)_buf, _buf_size).c_str(), _buf_size, n));
	std::string cheat;
	
	for(size_t i = 0; i < _cheats.size(); ++i) {
		const std::string &code = _cheats[i];
		if (std::string(_buf + _buf_size - code.size(), code.size()) == code) {
			LOG_DEBUG(("triggered cheat: %s", code.c_str()));
			cheat = code;
			break;
		}
	}
	if (cheat.empty())
		return false;
		
	if (cheat == "skotobaza") {
		/* SECRET ATATAT MODE !! */
		World->setMode("atatat", true);
	}
	return false;
}
