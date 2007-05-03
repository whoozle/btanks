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

#include "thread.h"
#include "sdl_ex.h"
#include "mrt/logger.h"
#include <assert.h>

int sdlx_thread_starter(void *o) {
	TRY {
		sdlx::Thread *t = reinterpret_cast<sdlx::Thread *>(o);
		assert(t != NULL);
		return t->runWrap();
	} CATCH("thread::run", );
	return -1;
}

using namespace sdlx;

const int Thread::runWrap() {
	_starter.post();
	return run();
}

Thread::Thread() : _thread(NULL) {}


Thread::~Thread() {
	if (_thread != NULL) 
		LOG_WARN(("~Thread: thread %x was not stopped", getID()));
}

Uint32 Thread::getID() const {
	if (_thread == NULL)
		throw_sdl(("getID: thread was not started"));
	
	return SDL_GetThreadID(_thread);
}


void Thread::start() {
	if (_thread != NULL) 
		throw_ex(("thread was already started."));
	_thread = SDL_CreateThread(sdlx_thread_starter, reinterpret_cast<void *>(this));
	_starter.wait();
}

const int Thread::wait() {
	if (_thread == NULL)
		throw_sdl(("wait: thread was not started"));
	int r;
	SDL_WaitThread(_thread, &r);
	_thread = NULL;
	return r;
}

void Thread::kill() {
	if (_thread == NULL)
		throw_sdl(("kill: thread was not started"));
	SDL_KillThread(_thread);
	_thread = NULL;
}
