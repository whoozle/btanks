#ifndef __SDLX__THREAD_H__
#define __SDLX__THREAD_H__

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


#include "sdlx.h"
#include <SDL_thread.h>
#include <string>
#include "semaphore.h"

int sdlx_thread_starter(void *o);

namespace sdlx {

class SDLXAPI Thread {
public:
	Thread();
	virtual ~Thread();

	void start(const std::string &name);

	Uint32 get_id() const;
	
	int wait();

protected: 
	virtual const int run() = 0;
	friend int ::sdlx_thread_starter(void *o);

private: 
	int runWrap();
	
	Thread(const Thread &);
	const Thread& operator=(const Thread &);
	SDL_Thread * _thread;
	sdlx::Semaphore _starter;
};

}

#endif
