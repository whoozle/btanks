
/* Battle Tanks Game
 * Copyright (C) 2006-2008 Battle Tanks team
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

/* 
 * Additional rights can be granted beyond the GNU General Public License 
 * on the terms provided in the Exception. If you modify this file, 
 * you may extend this exception to your version of the file, 
 * but you are not obligated to do so. If you do not wish to provide this
 * exception without modification, you must delete this exception statement
 * from your version and license this file solely under the GPL without exception. 
*/

#include "server.h"
#include "mrt/logger.h"
#include "mrt/exception.h"
#include "mrt/socket_set.h"
#include "player_manager.h"
#include "message.h"
#include "monitor.h"
#include "connection.h"
#include "config.h"
#include "rt_config.h"

#ifdef _WINDOWS
#	include <Winsock2.h>
#else
#	include <arpa/inet.h>
#endif              


Server::Server()  : _monitor(NULL), _sock() {}
Server::~Server() {
	delete _monitor;
	_monitor = NULL;
}

void Server::init() {
	GET_CONFIG_VALUE("multiplayer.bind-address", std::string, bindaddr, std::string());
	int port = RTConfig->port;
	GET_CONFIG_VALUE("multiplayer.compression-level", int, cl, 3);

	LOG_DEBUG(("starting game server at port %d", port));

	_udp_sock.listen(bindaddr, port, true);
	LOG_DEBUG(("udp socket started..."));
	_sock.listen(bindaddr, port, true);
	_sock.noDelay();

	_monitor = new Monitor(cl);
	_monitor->add(&_udp_sock);
	_monitor->add(&_sock);
	_monitor->start();
	
	if (RTConfig->server_mode) {
		GET_CONFIG_VALUE("multiplayer.server.register-on-master-server", bool, rms, true);
		GET_CONFIG_VALUE("multiplayer.server.master-server", std::string, mname, "btanks.media.netive.ru");
		GET_CONFIG_VALUE("multiplayer.server.master-server-port", int, mport, 27254);
		if (rms) {
			LOG_DEBUG(("registering server on master server..."));
			TRY {
				char buf[3];
				mrt::TCPSocket sock;
				sock.connect(mname, mport);
				
				buf[0] = 's';
				*((uint16_t *)(buf + 1)) = htons(port);
				int r = sock.send(buf, 3);
				LOG_DEBUG(("sent %d bytes...", r));
				//sock.recv(buf, 3);
			} CATCH("registering on master server", );
		}
	}
}

void Server::restart() {
	LOG_DEBUG(("Server::restart() called..."));
	std::queue<Connection *> conns;
	Connection *c;
	while((c = _monitor->pop()) != NULL)
		conns.push(c);
	
	while(!conns.empty()) {
		Connection *c = conns.front();
		conns.pop();
		TRY {
			Message msg(Message::RequestServerStatus);
			msg.set("release", RTConfig->release_name);
					
			int id = PlayerManager->on_connect();
			LOG_DEBUG(("reassigning connection: %d", id));
			_monitor->add(id, c);
			PlayerManager->on_message(id, msg);
			c = NULL;
		} CATCH("restart", { delete c;})
	}
}

void Server::disconnect_all() {
	Connection *c;
	while((c = _monitor->pop()) != NULL)
		delete c;
}

void Server::tick(const float dt) {
	if (!_monitor) 
		return;
	TRY {
		_monitor->accept();
	} CATCH("accepting client", );
	
	int id = -1;
	TRY {
		mrt::Chunk data;
		unsigned recv_ts;
		
		while(_monitor->recv(id, data)) {
			Message m;
			m.deserialize2(data);

			switch(m.type) {
			case Message::RequestServerStatus:
			case Message::PlayerState:
			case Message::Ping:
			case Message::Pong:
			case Message::RequestPlayer:	
			case Message::TextMessage:	
			case Message::PlayerMessage: 
			case Message::RequestObjects:
			case Message::JoinTeam:
				PlayerManager->on_message(id, m);

			case Message::ServerDiscovery:
				break;
			default:
				throw_ex(("message type %s is not allowed", m.getType()));
			}
		}

		while(_monitor->disconnected(id)) {
			PlayerManager->on_disconnect(id);
		}
	} CATCH("tick", {
		if (id >= 0) {
			disconnect(id);
		}
	});
}

void Server::send(const int id, const Message &m) {
	TRY {
		//LOG_DEBUG(("sending message '%s' to %d", m.getType(), id));
		mrt::Chunk data;
		m.serialize2(data);
		_monitor->send(id, data, m.realtime());
	} CATCH("send", {
		disconnect(id);
	});
}

void Server::broadcast(const Message &m) {
	TRY {
		LOG_DEBUG(("broadcasting message '%s'", m.getType()));
		mrt::Chunk data;
		m.serialize2(data);
		_monitor->broadcast(data, m.realtime());
	} CATCH("broadcast", {});
}


const bool Server::active() const { return _monitor->active(); }

void Server::disconnect(const int id) {
	_monitor->disconnect(id);
	PlayerManager->on_disconnect(id);
}
