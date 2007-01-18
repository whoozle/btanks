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

#include "socket_set.h"
#include "net_ex.h"
#include "tcp_socket.h"

using namespace sdlx;

SocketSet::SocketSet(const int max_sockets) {
	_set = SDLNet_AllocSocketSet(max_sockets);
	if (_set == NULL) 
		throw_net(("SDLNet_AllocSocketSet"));
}

void SocketSet::add(const sdlx::TCPSocket &sock) {
	if (sock._sock == NULL)
		throw_ex(("attempt to add uninitialized socket to set"));
	
	int numused = SDLNet_TCP_AddSocket(_set, sock._sock);
	if (numused == -1)
		throw_net(("SDLNet_TCP_AddSocket"));        
}

void SocketSet::add(const sdlx::TCPSocket *sock) {
	if (sock == NULL || sock->_sock == NULL)
		throw_ex(("attempt to add NULL/uninitialized socket to set"));
	
	int numused = SDLNet_TCP_AddSocket(_set, sock->_sock);
	if (numused == -1)
		throw_net(("SDLNet_TCP_AddSocket"));        
}


void SocketSet::remove(const sdlx::TCPSocket &sock) {
	if (sock._sock == NULL)
		throw_ex(("attempt to remove uninitialized socket from set"));

	int numused = SDLNet_TCP_DelSocket(_set, sock._sock);
	if (numused == -1)
		LOG_WARN(("SDLNet_TCP_DelSocket: %s", SDLNet_GetError()));	
}

const int SocketSet::check(const Uint32 timeout) const {
	int r = SDLNet_CheckSockets(_set, timeout);
	if (r == -1)
		throw_net(("SDLNet_CheckSockets"));
	return r;
}


SocketSet::~SocketSet() {
	SDLNet_FreeSocketSet(_set);
}

