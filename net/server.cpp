#include "server.h"
#include "mrt/logger.h"

Server::Server()  : _running(false) {}

void Server::init(const unsigned port) {
	LOG_DEBUG(("starting game server at port %d", port));
	_sock.listen(port);
	_running = true;
}

void Server::tick(const float dt) {
	if (!_running) 
		return;
		
	//send world coordinated, receive events.
}

