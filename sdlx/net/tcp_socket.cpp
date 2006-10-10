
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

#include "tcp_socket.h"
#include "net_ex.h"
#include "mrt/chunk.h"

using namespace sdlx;

TCPSocket::TCPSocket() : _sock(NULL) {}

void TCPSocket::listen(const unsigned port) {
	close();
	
	IPaddress ip;
	
	if (SDLNet_ResolveHost(&ip, NULL, port) == -1)
		throw_net(("SDLNet_ResolveHost"));
	        
	_sock = SDLNet_TCP_Open(&ip);
	if (!_sock) {
		throw_net(("SDLNet_TCP_Open"));
	}
}

void TCPSocket::connect(const std::string &host, const int port) {
	close();
	
	IPaddress ip;
	
	if(SDLNet_ResolveHost(&ip, host.c_str(), port)==-1) 
		throw_net(("SDLNet_ResolveHost"));
	        
	_sock = SDLNet_TCP_Open(&ip);
	if(!_sock) 
		throw_net(("SDLNet_TCP_Open"));	                
}

void TCPSocket::accept(sdlx::TCPSocket &client) {
	client.close();
	client._sock = SDLNet_TCP_Accept(_sock);
	if (client._sock == NULL)
		throw_net(("SDLNet_TCP_Accept"));
}

void TCPSocket::send(const void *data, const int len) const {
	int result = SDLNet_TCP_Send(_sock, (void *)data, len); //SDL_net const lack
	if (result < len) 
		throw_net(("SDLNet_TCP_Send"));
}

void TCPSocket::send(const mrt::Chunk &data) const {
	send(data.getPtr(), data.getSize());
}


const int TCPSocket::recv(void *data, const int len) const {
	int result = SDLNet_TCP_Recv(_sock, data, len);
	if (result <= 0) 
		throw_net(("SDLNet_TCP_Recv"));
	return result;
}


const bool TCPSocket::ready() const {
	return SDLNet_SocketReady(_sock) != 0;
}

void TCPSocket::close() {
	if (_sock == NULL) 
		return;
	SDLNet_TCP_Close(_sock);
	_sock = NULL;
}

TCPSocket::~TCPSocket() {
	close();
}
