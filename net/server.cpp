#include "server.h"
#include "mrt/logger.h"
#include "mrt/exception.h"
#include "sdlx/socket_set.h"

Server::Server()  : _running(false) {}

void Server::init(const unsigned port) {
	LOG_DEBUG(("starting game server at port %d", port));
	_sock.listen(port);
	_running = true;
}

void Server::tick(const float dt) {
	if (!_running) 
		return;
	TRY {
		//send world coordinated, receive events.
		sdlx::SocketSet set(1); //1 + players.
		set.add(_sock);
		set.check(0);
	
		if (_sock.ready()) {
			sdlx::TCPSocket s;
			_sock.accept(s);
			LOG_DEBUG(("client connected..."));
		}
	} CATCH("tick", {});
}

