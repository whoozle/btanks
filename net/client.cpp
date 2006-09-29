#include "mrt/logger.h"
#include "mrt/tcp_socket.h"
#include "mrt/exception.h"
#include "mrt/serializator.h"

#include "client.h"
#include "player_state.h"
#include "protocol.h"
#include "game.h"
#include "monitor.h"
#include "connection.h"


Client::Client(): _monitor(NULL) {
}

Client::~Client() {
	delete _monitor;
	_monitor = NULL;
}

void Client::init(const std::string &host, const unsigned port) {
	delete _monitor;

	LOG_DEBUG(("client::init('%s':%u)", host.c_str(), port));	
	Connection *conn = NULL;
	TRY { 
		conn = new Connection(new mrt::TCPSocket);
		conn->sock->connect(host, port);
		conn->sock->noDelay();
		_monitor = new Monitor;
		_monitor->start();
		_monitor->add(0, conn);
		conn = NULL;
	} CATCH("init", {delete conn; conn = NULL; throw; });
}

void Client::notify(const PlayerState &state) {
	if (!_monitor)
		return;
	
	Message m(Message::PlayerState);
	state.serialize2(m.data);
	send(m);
}

void Client::send(const Message &m) {
	LOG_DEBUG(("sending '%s'", m.getType()));

	mrt::Chunk data;
	m.serialize2(data);
	
	_monitor->send(0, data);	
}


void Client::tick(const float dt) {	
	if (_monitor == NULL) 
		return;

	int id;
	mrt::Chunk data;
	while(_monitor->recv(id, data)) {
		assert(id == 0);
		Message m;
		m.deserialize2(data);

		if (m.type != Message::UpdateWorld && m.type != Message::ServerStatus && 
			m.type != Message::UpdatePlayers && m.type != Message::Pang) 
			throw_ex(("message type '%s' is not allowed", m.getType()));
		Game->onMessage(0, m);
	}
	while(_monitor->disconnected(id)) {
		Game->onDisconnect(id);
	}
}

void Client::disconnect() {
	_monitor->disconnect(0);
	Game->onDisconnect(0);
}
