#ifndef __BTANKS_MRT_UDPSOCKET_H__
#define __BTANKS_MRT_UDPSOCKET_H__

/* M-runtime for c++
 * Copyright (C) 2005-2008 Vladimir Menshakov
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.

 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
*/


#include <string>
#include "sys_socket.h"
#include "export_mrt.h"

namespace mrt {
class Chunk;

class MRTAPI UDPSocket : public Socket {
public:
	UDPSocket();
	void connect(const mrt::Socket::addr &addr);
	void connect(const std::string &host, const int port);
	void listen(const std::string &addr, const unsigned port, const bool reuse = false);
	void set_broadcast_mode(int val = 1);

	const int send(const Socket::addr &addr, const void *data, const int len) const;
	void broadcast(const mrt::Chunk &data, const int port);
	const int recv(Socket::addr &addr, void *data, const int len) const;
};

}

#endif

