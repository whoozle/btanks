#include "server.h"
#include "mrt/logger.h"
#include "mrt/exception.h"
#include "sdlx/socket_set.h"
#include "game.h"
#include "protocol.h"
#include "player_state.h"
#include "connection.h"

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
				int id = Game->onConnect(msg);
				msg.send(*s);
				_connections.push_back(new Connection(s, id));
			} CATCH("accept", { delete s; s = NULL; })
		}
		for(ConnectionList::iterator i = _connections.begin(); i != _connections.end(); ) {
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
				TRY {
					int id = (*i)->id;
					LOG_DEBUG(("error, player %d disconnected", id));
					Game->onDisconnect(id);
				} CATCH("onDisconnect", {});
				delete *i;
				i = _connections.erase(i);
			} );
		}
	} CATCH("tick", {});
}

void Server::broadcast(const Message &m) {
	TRY {
		for(ConnectionList::iterator i = _connections.begin(); i != _connections.end(); ) {
			TRY {
				mrt::Serializator s;
				m.send(*(*i)->sock);
				++i;
			} CATCH("reading from socket", {
				TRY {
					int id = (*i)->id;
					LOG_DEBUG(("error, player %d disconnected", id));
					Game->onDisconnect(id);
				} CATCH("onDisconnect", {});
				delete *i;
				i = _connections.erase(i);
			} );
		}
	} CATCH("broadcast", {});
}


void Server::notify(const PlayerState &state) {
	if (!_running)
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
