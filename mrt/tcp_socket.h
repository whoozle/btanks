#ifndef __BTANKS_TCPSOCKET_H__
#define __BTANKS_TCPSOCKET_H__
/* Battle Tanks Game
 * Copyright (C) 2006 Battle Tanks team
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

#include <string>
#include "sys_socket.h"

namespace mrt {
class Chunk;

class TCPSocket : public Socket {
public:
	TCPSocket();
	void listen(const unsigned port);
	void connect(const std::string &host, const int port);
	const int send(const void *data, const int len) const;
	//void send(const mrt::Chunk &data) const;
	const int recv(void *data, const int len) const;
	
	void accept(TCPSocket &client);

	~TCPSocket();
	friend class SocketSet;
};

}

#endif

