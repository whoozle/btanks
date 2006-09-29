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
	void broadcast(const Message &m);
	void tick(const float dt);
	~Server();
	
	const bool active() const;
	void disconnect(const int id);
	
private:
	Monitor *_monitor;
	
	mrt::TCPSocket _sock;
};


#endif

