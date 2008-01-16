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


#include "semaphore.h"
#include "sdl_ex.h"

using namespace sdlx;

Semaphore::Semaphore(const Uint32 value) : _sem (SDL_CreateSemaphore(value)) {
	if (_sem == NULL) 
		throw_sdl(("SDL_CreateSemaphore"));
}

void Semaphore::post() {
	if (SDL_SemPost(_sem) == -1)
		throw_sdl(("SDL_SemPost"));
}

void Semaphore::wait() {
	if (SDL_SemWait(_sem) == -1)
		throw_sdl(("SDL_SemWait"));
}

const bool Semaphore::wait(const Uint32 timeout) {
	int r = SDL_SemWaitTimeout(_sem, timeout);
	switch(r) {
		case 0: return true;
		case SDL_MUTEX_TIMEDOUT: return false;
	}	
	throw_sdl(("SDL_SemWaitTimeout"));
	return false;
}

const bool Semaphore::tryWait() {
	int r = SDL_SemTryWait(_sem);
	switch(r) {
		case 0: return true;
		case SDL_MUTEX_TIMEDOUT: return false;
	}	
	throw_sdl(("SDL_SemTryWait"));
	return false;
}

Semaphore::~Semaphore() {
	SDL_DestroySemaphore(_sem);
	_sem = NULL;
}
