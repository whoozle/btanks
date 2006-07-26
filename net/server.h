#ifndef __BTANKS_SERVER_H__
#define __BTANKS_SERVER_H__

#include "sdlx/tcp_socket.h"
#include <deque>

class Server {
public:
	Server(); 
	void init(const unsigned port);
	void tick(const float dt);
	
private:
	struct Connection {
		Connection(sdlx::TCPSocket *s) : sock(s) {}
		~Connection() { delete sock; sock = NULL; }
		
		sdlx::TCPSocket * sock;
	};
	
	typedef std::deque<Connection *> ConnectionList;
	ConnectionList _connections;

	bool _running;
	sdlx::TCPSocket _sock;
};


#endif

