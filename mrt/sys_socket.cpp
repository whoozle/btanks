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
#include "sys_socket.h"
#include "ioexception.h"

#ifdef WIN32
#	include "Winsock2.h"
#else
#	include <sys/socket.h>
#	include <netinet/in.h>
#	include <arpa/inet.h>
#	include <unistd.h>
#	include <netdb.h>
#endif


using namespace mrt;

Socket::Socket() : _sock(-1) {}

const std::string Socket::addr::getName() const {
	struct hostent *he = gethostbyaddr((char *)&ip, sizeof(ip), AF_INET);
	if (he == NULL)
		return std::string();
	return he->h_name;
}

void Socket::addr::getAddr(const std::string &name) {
	struct hostent *he = gethostbyname(name.c_str());
	if (he == NULL || he->h_addrtype != AF_INET) //sorry, no ipv6 now
		return;
	ip = *((uint32_t*)he->h_addr_list[0]);
}

const std::string Socket::addr::getAddr() const {
	in_addr a;
	memset(&a, 0, sizeof(a));
	a.s_addr = ip;
	return inet_ntoa(a);
}

void Socket::init() {
	static bool inited;
	if (inited)
		return;
	
#ifdef WIN32
	struct WSAData WSAData;
	if (WSAStartup (MAKEWORD(1,1), &WSAData) != 0) 
		throw_ex(("failed to initialize winsock 1.1. upgrade your windows installation and/or upgrade mrt runtime"));
#endif
	inited = true;
}

void Socket::create(const int af, int type, int protocol) {
	init();
	close();
	
	_sock = socket(af, type, protocol);
	if (_sock == -1) 
		throw_io(("socket"));
}

void Socket::close() {
	if (_sock == -1)
		return;
	
	shutdown(_sock, 2);
#ifdef WIN32
	::closesocket(_sock);
#else
	::close(_sock);
#endif
	_sock = -1;
}

Socket::~Socket() {
	close();
}
