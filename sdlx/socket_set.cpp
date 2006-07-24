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

