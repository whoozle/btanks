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
	void send(const int id, const Message &m);
	void tick(const float dt);
	~Server();
	
private:
	Monitor *_monitor;
	
	mrt::TCPSocket _sock;
};


#endif

