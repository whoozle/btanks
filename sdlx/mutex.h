#ifndef __SDLX_MUTEX_H__
#define __SDLX_MUTEX_H__

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

namespace sdlx {

class SDLXAPI Mutex {
public: 
	Mutex();
	~Mutex();
private: 
	Mutex(const Mutex &);
	const Mutex& operator=(const Mutex &);
	
	void lock() const;
	void unlock() const;

	SDL_mutex *_mutex;
	
	friend class AutoMutex;
};

class SDLXAPI AutoMutex {
public: 
	AutoMutex(const Mutex &m, const bool lock = true);
	void lock() const;
	void unlock() const;
	~AutoMutex();
private:
	const Mutex &_mutex;
	mutable bool _locked;
};

}

#endif

