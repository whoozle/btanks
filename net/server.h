#ifndef __BTANKS_SERVER_H__
#define __BTANKS_SERVER_H__

#include "mrt/tcp_socket.h"
#include <deque>

class Connection;
class PlayerState;
class Message;

class Server {
public:
	Server(); 
	void init(const unsigned port);
	void notify(const PlayerState &state);
	void tick(const float dt);
	void broadcast(const Message &m);
	
private:
	
	typedef std::deque<Connection *> ConnectionList;
	ConnectionList _connections;

	bool _running;
	mrt::TCPSocket _sock;
};


#endif

