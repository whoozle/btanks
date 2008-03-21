#ifndef CLUNK_LOCKER_H__
#define CLUNK_LOCKER_H__

/* libclunk - realtime 2d/3d sound render library
 * Copyright (C) 2005-2008 Netive Media Group
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



#include <SDL_audio.h>
#include "export_clunk.h"

struct CLUNKAPI AudioLocker {
	AudioLocker () {
		SDL_LockAudio();
	}
	~AudioLocker() {
		SDL_UnlockAudio();
	}
};


#endif

