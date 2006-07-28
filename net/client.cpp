#include "client.h"
#include "mrt/logger.h"
#include "player_state.h"
#include "protocol.h"
#include "sdlx/socket_set.h"
#include "game.h"
#include "connection.h"
#include "sdlx/tcp_socket.h"

Client::Client():  _conn(NULL), _running(false) {}

void Client::init(const std::string &host, const unsigned port) {
	LOG_DEBUG(("connecting to %s:%u [stub]", host.c_str(), port));
	
	delete _conn;
	_conn = new Connection(new sdlx::TCPSocket);
	_conn->sock->connect(host, port);
	_running = true;
}

void Client::notify(const PlayerState &state) {
	if (!_running)
		return;
	
	LOG_DEBUG(("notify from player"));
	Message m(PlayerEvent);
	m.data[0] = state.left?1:0 | state.right?2:0 | state.up ? 4:0 | state.down ? 8:0 | state.fire ? 16:0;
	m.data_size = 1;

	m.send(*_conn->sock);	
}

void Client::tick(const float dt) {	
	sdlx::SocketSet set(1);
	set.add(_conn->sock);
	set.check(0);
	if (_conn->sock->ready()) {
		Message m;
		m.recv(*_conn->sock);
		if (m.type != UpdateWorld && m.type != ServerStatus) 
			throw_ex(("message type %d is not allowed", m.type));
		Game->onMessage(*_conn, m);
	}
}
