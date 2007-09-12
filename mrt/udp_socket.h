#ifndef __BTANKS_MRT_UDPSOCKET_H__
#define __BTANKS_MRT_UDPSOCKET_H__

/* M-runtime for c++
 * Copyright (C) 2005-2007 Vladimir Menshakov
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
#include "export_mrt.h"

namespace mrt {
class Chunk;

class MRTAPI UDPSocket : public Socket {
public:
	void create();
	void listen(const std::string &addr, const unsigned port, const bool reuse = false);

	const int send(const Socket::addr &addr, const void *data, const int len) const;
	const int recv(Socket::addr &addr, void *data, const int len) const;
};

}

#endif

