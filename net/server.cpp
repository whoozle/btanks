
/* Battle Tanks Game
 * Copyright (C) 2006-2007 Battle Tanks team
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

#include "server.h"
#include "mrt/logger.h"
#include "mrt/exception.h"
#include "mrt/socket_set.h"
#include "player_manager.h"
#include "protocol.h"
#include "monitor.h"
#include "connection.h"

Server::Server()  : _monitor(NULL), _sock() {}
Server::~Server() {
	delete _monitor;
	_monitor = NULL;
}

void Server::init(const unsigned port) {
	LOG_DEBUG(("starting game server at port %d", port));
	_udp_sock.listen(port);
	LOG_DEBUG(("udp socket started..."));
	_sock.listen(port, true);
	_sock.noDelay();

	_monitor = new Monitor;
	_monitor->add(&_udp_sock);
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
		
		int socks_n = set.check(0);
		if (socks_n > 0 && set.check(_sock, mrt::SocketSet::Read)) {
			mrt::TCPSocket *s = NULL;
			TRY {
				s = new mrt::TCPSocket;
				_sock.accept(*s);
				s->noDelay();
				
				LOG_DEBUG(("client connected..."));
				Message msg(Message::ServerStatus);
				int id = PlayerManager->onConnect(msg);
				_monitor->add(id, new Connection(s));
				send(id, msg);
			} CATCH("accept", { delete s; s = NULL; })
		}

		mrt::Chunk data;
		int delta;
		
		while(_monitor->recv(id, data, delta)) {
			Message m;
			m.deserialize2(data);

			switch(m.type) {
			case Message::PlayerState:
			case Message::Ping:
			case Message::Pong:
			case Message::RequestPlayer:	
			case Message::TextMessage:	
			case Message::PlayerMessage: 
				PlayerManager->onMessage(id, m, delta);
				break;
			default:
				throw_ex(("message type %s is not allowed", m.getType()));
			}
		}

		while(_monitor->disconnected(id)) {
			PlayerManager->onDisconnect(id);
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
	} CATCH("send", {
		disconnect(id);
	});
}

void Server::broadcast(const Message &m) {
	TRY {
		LOG_DEBUG(("broadcasting message '%s'", m.getType()));
		mrt::Chunk data;
		m.serialize2(data);
		_monitor->broadcast(data);
	} CATCH("broadcast", {});
}


const bool Server::active() const { return _monitor->active(); }

void Server::disconnect(const int id) {
	_monitor->disconnect(id);
	PlayerManager->onDisconnect(id);
}
