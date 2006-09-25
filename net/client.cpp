#include "mrt/logger.h"
#include "mrt/socket_set.h"
#include "mrt/tcp_socket.h"
#include "mrt/exception.h"
#include "mrt/serializator.h"

#include "client.h"
#include "player_state.h"
#include "protocol.h"
#include "game.h"
#include "connection.h"


Client::Client():  _conn(NULL), _running(false) {}

void Client::init(const std::string &host, const unsigned port) {
	LOG_DEBUG(("connecting to %s:%u [stub]", host.c_str(), port));
	
	delete _conn;
	_conn = new Connection(new mrt::TCPSocket);
	_conn->sock->connect(host, port);
	_running = true;
}

void Client::notify(const PlayerState &state) {
	if (!_running)
		return;
	
	//LOG_DEBUG(("notify from player"));
	Message m(PlayerEvent);
	mrt::Serializator s;
	state.serialize(s);
	m.data = s.getData();

	m.send(*_conn->sock);	
}

void Client::tick(const float dt) {	
	mrt::SocketSet set;
	set.add(_conn->sock);
	set.check(0);
	if (set.check(_conn->sock,mrt::SocketSet::Read)) {
		Message m;
		m.recv(*_conn->sock);
		if (m.type != UpdateWorld && m.type != ServerStatus) 
			throw_ex(("message type %d is not allowed", m.type));
		Game->onMessage(*_conn, m);
	}
}
