/* sdlx - c++ wrapper for libSDL
 * Copyright (C) 2005-2007 Vladimir Menshakov
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

#include "mutex.h"
#include "sdl_ex.h"

using namespace sdlx;

Mutex::Mutex() : _mutex(SDL_CreateMutex()) {
	if (_mutex == NULL)
		throw_sdl(("SDL_CreateMutex"));
}

Mutex::~Mutex() {
	SDL_DestroyMutex(_mutex);
	_mutex = NULL;
}

void Mutex::lock() const {
	if (SDL_mutexP(_mutex) != 0)
		throw_sdl(("SDL_LockMutex"));
}

void Mutex::unlock() const {
	if (SDL_mutexV(_mutex) != 0)
		throw_sdl(("SDL_UnlockMutex"));
}


AutoMutex::AutoMutex(const Mutex &m, const bool lock) : _mutex(m), _locked(lock) {
	if (lock)
		_mutex.lock();
}

void AutoMutex::lock() const {
	_mutex.lock();
	_locked = true;
}

void AutoMutex::unlock() const {
	_mutex.unlock();
	_locked = false;
}

AutoMutex::~AutoMutex() {
	if (_locked)
		_mutex.unlock();
}
