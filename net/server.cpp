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
	_sock.listen(port);
	_monitor = new Monitor;
	_monitor->start();
}

void Server::tick(const float dt) {
	if (!_monitor) 
		return;
	TRY {
		//send world coordinated, receive events.
		mrt::SocketSet set;
		set.add(_sock);
		
		if (set.check(0) == 0)
			return;
	
		if (set.check(_sock, mrt::SocketSet::Read)) {
			mrt::TCPSocket *s = NULL;
			TRY {
				s = new mrt::TCPSocket;
				_sock.accept(*s);
				LOG_DEBUG(("client connected..."));
				Message msg(ServerStatus);
				int id = Game->onConnect(msg);
				_monitor->add(id, new Connection(s));
				mrt::Chunk data;
				msg.serialize2(data);
				
				_monitor->send(id, data);
			} CATCH("accept", { delete s; s = NULL; })
		}
		mrt::Chunk data;
		int id;
		if (_monitor->recv(id, data)) {
			Message m;
			m.deserialize2(data);
			if (m.type != PlayerEvent) 
				throw_ex(("message type %d is not allowed", m.type));
	
			Game->onMessage(id, m);
		}
	} CATCH("tick", {});
}

void Server::broadcast(const Message &m) {
	TRY {
		mrt::Chunk data;
		m.serialize2(data);
		_monitor->broadcast(data);
	} CATCH("broadcast", {});
}


void Server::notify(const PlayerState &state) {
	if (!_monitor)
		return;
	
/*	LOG_DEBUG(("notify my state to clients"));
	Message m(PlayerEvent);
	m.data.setSize(1);
	m.data[0] = state.left?1:0 | state.right?2:0 | state.up ? 4:0 | state.down ? 8:0 | state.fire ? 16:0;
*/
/*	for(ConnectionList::iterator i = _connections.begin(); i != _connections.end(); ) {
			TRY {
				if ((*i)->sock->ready()) {
					LOG_DEBUG(("event in connection %p", (void *)*i));
					Message m;
					m.recv(*(*i)->sock);
					if (m.type != PlayerEvent) 
						throw_ex(("message type %d is not allowed", m.type));
					
					Game->onMessage(**i, m);
				}
				++i;
			} CATCH("reading from socket", {
				LOG_DEBUG(("error, client disconnected"));
				delete *i;
				i = _connections.erase(i);
			} );
		}
*/
}
