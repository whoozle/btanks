
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

#include "mrt/logger.h"
#include "mrt/tcp_socket.h"
#include "mrt/exception.h"
#include "mrt/serializator.h"

#include "client.h"
#include "rt_config.h"
#include "player_state.h"
#include "message.h"
#include "player_manager.h"
#include "monitor.h"
#include "connection.h"
#include "config.h"

Client::Client(): _monitor(NULL), sent_req(false) {
}

Client::~Client() {
	delete _monitor;
	_monitor = NULL;
}

void Client::init(const mrt::Socket::addr &host) {
	delete _monitor;

	GET_CONFIG_VALUE("multiplayer.compression-level", int, cl, 3);
	
	LOG_DEBUG(("client::init('%s')", host.getAddr().c_str()));
	//_udp_sock.create();
	//_udp_sock.listen(bindaddr, port);
	_monitor = new Monitor(cl);
	_monitor->add(&_udp_sock);
	_monitor->connect(host);
	_monitor->start();
	sent_req = false;
}

void Client::send(const Message &m) {
	LOG_DEBUG(("sending '%s' via channel %d", m.getType(), m.channel));

	mrt::Chunk data;
	m.serialize2(data);
	
	_monitor->send(0, data, m.realtime());	
}


void Client::tick(const float dt) {	
	if (_monitor == NULL) 
		return;
		
	if (!sent_req && connected()) {
		Message msg(Message::RequestServerStatus);
		msg.set("release", RTConfig->release_name);
		send(msg);
		sent_req = true;
	}

	int id;
	mrt::Chunk data;
	while(_monitor->recv(id, data)) {
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
		case Message::ServerError:
			PlayerManager->on_message(0, m);
			break;

		default:
			throw_ex(("message type '%s' is not allowed", m.getType()));
		}
	}
	while(_monitor->disconnected(id)) {
		PlayerManager->on_disconnect(id);
	}
}

void Client::disconnect() {
	_monitor->disconnect(0);
	PlayerManager->on_disconnect(0);
	sent_req = false;
}

bool Client::connected() const {
	return _monitor->connected(0);
}
