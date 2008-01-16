#ifndef __BTANKS_MRT_TCPSOCKET_H__
#define __BTANKS_MRT_TCPSOCKET_H__

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

class MRTAPI TCPSocket : public Socket {
public:
	TCPSocket();
	
	void noDelay(const bool value = true);
	
	void listen(const std::string &addr, const unsigned port, const bool reuse = false);
	void connect(const std::string &host, const int port, const bool no_delay = false);
	const int send(const void *data, const int len) const;
	//void send(const mrt::Chunk &data) const;
	const int recv(void *data, const int len) const;
	
	void accept(TCPSocket &client);
	inline const Socket::addr getAddress() const { return _addr; }
private: 
	Socket::addr _addr;
};

}

#endif

