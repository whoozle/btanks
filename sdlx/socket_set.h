#ifndef __SDLX_NET_SOCKET_SET_H__
#define __SDLX_NET_SOCKET_SET_H__

#include <SDL/SDL_net.h>

namespace sdlx {
class TCPSocket;
class SocketSet {
public: 
	SocketSet(const int max_sockets);
	void add(const sdlx::TCPSocket &sock);
	void remove(const sdlx::TCPSocket &sock);
	
	const int check(const Uint32 timeout) const;
	
	~SocketSet();
protected: 
	SDLNet_SocketSet _set;	
};
}

#endif

