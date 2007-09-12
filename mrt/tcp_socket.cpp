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
#include "tcp_socket.h"
#include <sys/types.h>
#include <string.h>


#ifdef WIN32
#	include "Winsock2.h"
#else
#	include <sys/socket.h>
#	include <netinet/in.h>
#	include <netinet/tcp.h>
#	include <netinet/ip.h> /* superset of previous */
#	include <arpa/inet.h>
#	include <netdb.h>
#endif              

#ifdef WIN32
#	ifndef socklen_t 
#		define socklen_t int
#	endif

#	ifndef in_addr_t
#		define in_addr_t unsigned long
#	endif
#endif


#include "ioexception.h"              

using namespace mrt;

TCPSocket::TCPSocket() {}

void TCPSocket::listen(const unsigned port, const bool reuse) {
	create(PF_INET, SOCK_STREAM, 0);

	int on = 1;
	if (reuse)
		setsockopt(_sock, SOL_SOCKET, SO_REUSEADDR, (char*)&on, sizeof(on));

	struct sockaddr_in addr;
	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	
	if (bind(_sock, (const sockaddr *)&addr, sizeof(addr)) == -1)
		throw_io(("bind"));
	
	if (::listen(_sock, 10) == -1)
		throw_io(("listen"));
}

void TCPSocket::connect(const std::string &host, const int port, const bool no_delay) {
	create(PF_INET, SOCK_STREAM, 0);
	if (no_delay)
		noDelay();

	struct sockaddr_in addr;
	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	addr.sin_addr.s_addr = inet_addr(host.c_str());

	//add gethostbyname here.
	if (addr.sin_addr.s_addr == (in_addr_t)-1) {
		//try to resolve host
		hostent *he = gethostbyname(host.c_str());
		if (he == NULL)
			throw_ex(("host '%s' was not found", host.c_str()));
		addr.sin_addr = *(struct in_addr*)(he->h_addr_list[0]);
	}
	
	LOG_DEBUG(("connect %s:%d", inet_ntoa(addr.sin_addr), port));
	if (::connect(_sock, (const struct sockaddr*)&addr, sizeof(addr))	 == -1)
		throw_io(("connect"));

	_addr.ip = addr.sin_addr.s_addr;
	_addr.port = port;
}

const int TCPSocket::send(const void *data, const int len) const {
#ifdef WIN32
	return ::send(_sock, (const char *)data, len, 0);
#else
	return ::send(_sock, data, len, 0);
#endif
}
//void send(const mrt::Chunk &data) const;
const int TCPSocket::recv(void *data, const int len) const {
#ifdef WIN32
	return ::recv(_sock, (char *)data, len, 0);
#else
	return ::recv(_sock, data, len, 0);
#endif
}

void TCPSocket::accept(TCPSocket &client) {
	struct sockaddr_in addr;
	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	socklen_t len = sizeof(addr);
	int s = ::accept(_sock, (struct sockaddr *)&addr, &len);
	if (s == -1)
		throw_io(("accept"));
	client.close();
	client._sock = s;

	client._addr.ip = addr.sin_addr.s_addr;
	client._addr.port = ntohs(addr.sin_port); //real port
}

#ifdef WIN32
#ifndef IP_TOS
#	define IP_TOS              3
#endif

#ifndef IPTOS_LOWDELAY
#	define IPTOS_LOWDELAY		0x10
#	define IPTOS_THROUGHPUT		0x08
#	define IPTOS_RELIABILITY	0x04
#endif
#endif

void TCPSocket::noDelay(const bool flag) {
TRY {
	if (_sock == -1)
		throw_ex(("noDelay on unitialized socket"));

	int value = flag?1:0;
	int r = setsockopt(_sock, IPPROTO_TCP, TCP_NODELAY, (char *)&value, sizeof(value));
	if (r < 0) 
		throw_io(("setsockopt(TCP_NODELAY)"));

#ifndef WIN32
	if (flag) {	
		value = IPTOS_LOWDELAY;
		r = setsockopt(_sock, IPPROTO_IP, IP_TOS, (char *)&value, sizeof(value));
		if (r < 0) 
			throw_io(("setsockopt(TOS_LOWDELAY)"));
	}
#endif
} CATCH("noDelay", {});
}
