#ifndef __BTANKS_SERVER_H__
#define __BTANKS_SERVER_H__

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

#include "export_net.h"
#include "mrt/tcp_socket.h"
#include <deque>

class PlayerState;
class Message;
class Monitor;

class BTNETAPI Server {
public:
	Server(); 
	void init(const unsigned port);
	void send(const int id, const Message &m);
	void broadcast(const Message &m);
	void tick(const float dt);
	~Server();
	
	const bool active() const;
	void disconnect(const int id);
	
private:
	Server(const Server &);
	const Server& operator=(const Server &);
	Monitor *_monitor;
	
	mrt::TCPSocket _sock;
};


#endif

