#ifndef __BTANKS_SERVER_H__
#define __BTANKS_SERVER_H__

#include "mrt/tcp_socket.h"
#include <deque>

class PlayerState;
class Message;
class Monitor;

class Server {
public:
	Server(); 
	void init(const unsigned port);
	void notify(const PlayerState &state);
	void tick(const float dt);
	void broadcast(const Message &m);
	~Server();
	
private:
	Monitor *_monitor;
	
	mrt::TCPSocket _sock;
};


#endif

