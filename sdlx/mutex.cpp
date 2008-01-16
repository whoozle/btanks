/* sdlx - c++ wrapper for libSDL
 * Copyright (C) 2005-2007 Vladimir Menshakov
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.

 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
*/


#include "mutex.h"
#include "sdl_ex.h"

using namespace sdlx;

Mutex::Mutex() : _mutex(NULL) {
	_mutex = SDL_CreateMutex();
	if (_mutex == NULL)
		throw_sdl(("SDL_CreateMutex"));
}

Mutex::~Mutex() {
	if (_mutex == NULL)
		return;
	
	SDL_DestroyMutex(_mutex);
	_mutex = NULL;
}

void Mutex::lock() const {
	if (_mutex == NULL)
		throw_ex(("lock() called on uninitialized mutex"));
	if (SDL_mutexP(_mutex) != 0)
		throw_sdl(("SDL_LockMutex"));
}

void Mutex::unlock() const {
	if (_mutex == NULL)
		throw_ex(("unlock() called on uninitialized mutex"));
	if (SDL_mutexV(_mutex) != 0)
		throw_sdl(("SDL_UnlockMutex"));
}


AutoMutex::AutoMutex(const Mutex &m, const bool lock) : _mutex(m), _locked(lock) {
	if (lock)
		_mutex.lock();
}

void AutoMutex::lock() const {
	if (_locked)
		throw_ex(("lock called on locked automutex"));
	_mutex.lock();
	_locked = true;
}

void AutoMutex::unlock() const {
	if (!_locked)
		throw_ex(("unlock called on unlocked automutex"));
	_mutex.unlock();
	_locked = false;
}

AutoMutex::~AutoMutex() {
	if (_locked)
		_mutex.unlock();
}
