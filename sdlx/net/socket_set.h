#ifndef __SDLX_NET_SOCKET_SET_H__
#define __SDLX_NET_SOCKET_SET_H__

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

#include <SDL/SDL_net.h>

namespace sdlx {
class TCPSocket;
class SocketSet {
public: 
	SocketSet(const int max_sockets);
	void add(const sdlx::TCPSocket &sock);
	void add(const sdlx::TCPSocket *sock);
	void remove(const sdlx::TCPSocket &sock);
	
	const int check(const Uint32 timeout) const;
	
	~SocketSet();
protected: 
	SDLNet_SocketSet _set;	
};
}

#endif

