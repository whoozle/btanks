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
	
	//LOG_DEBUG(("notify from player"));
	mrt::Chunk data;
	Message m(PlayerEvent);
	state.serialize2(m.data);
	m.serialize2(data);
	
	_monitor->send(0, data);
}

void Client::tick(const float dt) {	
	if (_monitor == NULL) 
		return;
	
	int id;
	mrt::Chunk data;
	if (_monitor->recv(id, data)) {
		assert(id == 0);
		Message m;
		m.deserialize2(data);
		if (m.type != UpdateWorld && m.type != ServerStatus) 
			throw_ex(("message type %d is not allowed", m.type));
		Game->onMessage(0, m);
	}
}
