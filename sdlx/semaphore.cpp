
/* Battle Tanks Game
 * Copyright (C) 2006 Battle Tanks team
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

#include "semaphore.h"
#include "sdl_ex.h"

using namespace sdlx;

Semaphore::Semaphore(const Uint32 value) {
	if ((_sem = SDL_CreateSemaphore(value)) == NULL) 
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
