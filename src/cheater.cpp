
/* Battle Tanks Game
 * Copyright (C) 2006-2007 Battle Tanks team
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
#include "cheater.h"
#include "window.h"
#include <string.h>
#include <assert.h>

Cheater::Cheater() : _buf_size(0) {
	Window->key_signal.connect(sigc::mem_fun(this, &Cheater::onKey));
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

bool Cheater::onKey(const SDL_keysym sym, const bool pressed) {
	if (!pressed)
		return false;
	
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
		if (code.size() >= _buf_size && strncmp(_buf + _buf_size - code.size(), code.c_str(), code.size()) == 0) {
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
