#ifndef __BTANKS_SERVER_H__
#define __BTANKS_SERVER_H__

#include "sdlx/tcp_socket.h"

class Server {
public:
	Server(); 
	void init(const unsigned port);
	void tick(const float dt);
	
private:
	bool _running;
	sdlx::TCPSocket _sock;
};


#endif

