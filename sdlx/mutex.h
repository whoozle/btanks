#ifndef __SDLX_MUTEX_H__
#define __SDLX_MUTEX_H__

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


#include "sdlx.h"
#include <SDL/SDL_thread.h>

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

