#include "server.h"
#include "mrt/logger.h"
#include "mrt/exception.h"
#include "mrt/socket_set.h"
#include "game.h"
#include "protocol.h"
#include "player_state.h"
#include "monitor.h"
#include "connection.h"

Server::Server()  : _monitor(0) {}
Server::~Server() {
	delete _monitor;
	_monitor = NULL;
}

void Server::init(const unsigned port) {
	LOG_DEBUG(("starting game server at port %d", port));
	_sock.listen(port, true);
	_monitor = new Monitor;
	_monitor->start();
}

void Server::tick(const float dt) {
	if (!_monitor) 
		return;
	int id = -1;
	TRY {
		//send world coordinated, receive events.
		mrt::SocketSet set;
		set.add(_sock);
		
		if (set.check(0) > 0 && set.check(_sock, mrt::SocketSet::Read)) {
			mrt::TCPSocket *s = NULL;
			TRY {
				s = new mrt::TCPSocket;
				_sock.accept(*s);
				s->noDelay();
				
				LOG_DEBUG(("client connected..."));
				Message msg(Message::ServerStatus);
				int id = Game->onConnect(msg);
				_monitor->add(id, new Connection(s));
				send(id, msg);
			} CATCH("accept", { delete s; s = NULL; })
		}

		mrt::Chunk data;
		
		while(_monitor->recv(id, data)) {
			Message m;
			m.deserialize2(data);
			
			if (m.type != Message::PlayerState && m.type != Message::Ping && m.type != Message::Pong) 
				throw_ex(("message type %s is not allowed", m.getType()));
	
			Game->onMessage(id, m);
		}

		while(_monitor->disconnected(id)) {
			Game->onDisconnect(id);
		}
	} CATCH("tick", {
		if (id >= 0) {
			disconnect(id);
		}
	});
}

void Server::send(const int id, const Message &m) {
	TRY {
		LOG_DEBUG(("sending message '%s' to %d", m.getType(), id));
		mrt::Chunk data;
		m.serialize2(data);
		_monitor->send(id, data);
	} CATCH("send", throw;);
}

void Server::broadcast(const Message &m) {
	TRY {
		LOG_DEBUG(("broadcasting message '%s'", m.getType()));
		mrt::Chunk data;
		m.serialize2(data);
		_monitor->broadcast(data);
	} CATCH("broadcast", throw;);
}


const bool Server::active() const { return _monitor->active(); }

void Server::disconnect(const int id) {
	_monitor->disconnect(id);
	Game->onDisconnect(id);
}
