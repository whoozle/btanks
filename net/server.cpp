#include "server.h"
#include "mrt/logger.h"
#include "mrt/exception.h"
#include "sdlx/socket_set.h"
#include "game.h"
#include "protocol.h"

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
		sdlx::SocketSet set(1 + _connections.size()); //1 + players.
		set.add(_sock);
		for(ConnectionList::iterator i = _connections.begin(); i != _connections.end(); ++i) {
			set.add(*(*i)->sock);
		}
		set.check(0);
	
		if (_sock.ready()) {
			sdlx::TCPSocket *s = NULL;
			TRY {
				s = new sdlx::TCPSocket;
				_sock.accept(*s);
				LOG_DEBUG(("client connected..."));
				Message msg(ServerStatus);
				Game->onClient(msg);
				_connections.push_back(new Connection(s));
			} CATCH("accept", { delete s; s = NULL; })
		}
		for(ConnectionList::iterator i = _connections.begin(); i != _connections.end(); ++i) {
			if ((*i)->sock->ready()) {
				LOG_DEBUG(("event in connection %p", (void *)*i));
				Message m;
				m.recv(*(*i)->sock);
			}
		}
	} CATCH("tick", {});
}

