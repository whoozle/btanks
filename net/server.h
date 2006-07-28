#ifndef __BTANKS_SERVER_H__
#define __BTANKS_SERVER_H__

#include "sdlx/tcp_socket.h"
#include <deque>

class Connection;
class Server {
public:
	Server(); 
	void init(const unsigned port);
	void tick(const float dt);
	
private:
	
	typedef std::deque<Connection *> ConnectionList;
	ConnectionList _connections;

	bool _running;
	sdlx::TCPSocket _sock;
};


#endif

