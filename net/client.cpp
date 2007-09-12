
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

#include "mrt/logger.h"
#include "mrt/tcp_socket.h"
#include "mrt/exception.h"
#include "mrt/serializator.h"

#include "client.h"
#include "player_state.h"
#include "protocol.h"
#include "player_manager.h"
#include "monitor.h"
#include "connection.h"
#include "config.h"

Client::Client(): _monitor(NULL) {
}

Client::~Client() {
	delete _monitor;
	_monitor = NULL;
}

void Client::init(const std::string &host) {
	delete _monitor;

	GET_CONFIG_VALUE("multiplayer.bind-address", std::string, bindaddr, std::string());
	GET_CONFIG_VALUE("multiplayer.port", int, port, 9876);
	
	LOG_DEBUG(("client::init('%s':%u)", host.c_str(), port));	
	_udp_sock.listen(bindaddr, port);
	LOG_DEBUG(("udp socket started..."));

	Connection *conn = NULL;
	TRY { 
		conn = new Connection(new mrt::TCPSocket);
		conn->sock->connect(host, port, true);
		conn->sock->noDelay();
		_monitor = new Monitor;
		_monitor->add(&_udp_sock);
		_monitor->start();
		_monitor->add(0, conn);
		conn = NULL;
	} CATCH("init", {delete conn; conn = NULL; throw; });
}

/*
void Client::notify(const PlayerState &state) {
	if (!_monitor)
		return;
	
	Message m(Message::PlayerState);
	state.serialize2(m.data);
	send(m);
}
*/

void Client::send(const Message &m) {
	LOG_DEBUG(("sending '%s' via channel %d", m.getType(), m.channel));

	mrt::Chunk data;
	m.serialize2(data);
	
	_monitor->send(0, data);	
}


void Client::tick(const float dt) {	
	if (_monitor == NULL) 
		return;

	int id;
	mrt::Chunk data;
	int delta;
	while(_monitor->recv(id, data, delta)) {
		assert(id == 0);
		Message m;
		m.deserialize2(data);

		switch(m.type) {
		case Message::UpdateWorld:
		case Message::ServerStatus:
		case Message::UpdatePlayers:
		case Message::Pang:
		case Message::Respawn:
		case Message::GameJoined:
		case Message::GameOver:
		case Message::TextMessage:
		case Message::DestroyMap:
		case Message::PlayerMessage:
			PlayerManager->onMessage(0, m, delta);
			break;

		default:
			throw_ex(("message type '%s' is not allowed", m.getType()));
		}
	}
	while(_monitor->disconnected(id)) {
		PlayerManager->onDisconnect(id);
	}
}

void Client::disconnect() {
	_monitor->disconnect(0);
	PlayerManager->onDisconnect(0);
}
