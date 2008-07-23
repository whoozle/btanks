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

#include "sys_socket.h"
#include "net_exception.h"
#include "serializator.h"
#include <string.h>
#include "fmt.h"

#ifdef _WINDOWS
#	include "Winsock2.h"
#else
#	include <sys/socket.h>
#	include <netinet/in.h>
#	include <arpa/inet.h>
#	include <unistd.h>
#	include <netdb.h>
#endif

#include <assert.h>
#include <stdlib.h>

using namespace mrt;

Socket::Socket() : _sock(-1) {}

const std::string Socket::addr::getName() const {
	struct hostent *he = gethostbyaddr((char *)&ip, sizeof(ip), AF_INET);
	if (he == NULL)
		return std::string();
	return he->h_name;
}

void Socket::addr::getAddrByName(const std::string &name) {
	struct hostent *he = gethostbyname(name.c_str());
	if (he == NULL || he->h_addrtype != AF_INET) //sorry, no ipv6 now
		return;
	ip = *((mrt_uint32_t*)he->h_addr_list[0]);
}

void Socket::addr::parse(const std::string &ip) {
	std::vector<std::string> ipport;
	mrt::split(ipport, ip, ":");
	if (ipport.empty()) {
		this->ip = 0;
		this->port = 0;
		return;
	}
	if (ipport.size() > 1) {
		port = atoi(ipport[1].c_str());
	}
#ifdef _WINDOWS
	this->ip = inet_addr(ipport[0].c_str());
	if (this->ip == INADDR_NONE)
		this->ip = 0;
#else	
	struct in_addr a;
	this->ip = (inet_aton(ipport[0].c_str(), &a) != -1)? a.s_addr: 0;
#endif
}

const std::string Socket::addr::getAddr(bool with_port) const {
	in_addr a;
	memset(&a, 0, sizeof(a));
	a.s_addr = ip;
	std::string r = inet_ntoa(a);
	if (with_port && port != 0) {
		r += mrt::format_string(":%u", port);
	}
	return r;
}

void Socket::init() {
	static bool inited;
	if (inited)
		return;
	
#ifdef _WINDOWS
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
		throw_net(("socket"));

	no_linger();
}

void Socket::close() {
	if (_sock == -1)
		return;
	
	//shutdown(_sock, 2); //we use do not use lingering by default
#ifdef _WINDOWS
	::closesocket(_sock);
#else
	::close(_sock);
#endif
	_sock = -1;
}

Socket::~Socket() {
	close();
}

void Socket::no_linger() {
TRY {
	struct linger lflags;
	memset(&lflags, 0, sizeof(lflags)); //disable linger
	int r = setsockopt(_sock, SOL_SOCKET, SO_LINGER, (char *)&lflags, sizeof(lflags));
	if (r < 0) 
		throw_net(("setsockopt(SO_LINGER)"));
} CATCH("noLinger", {});
}

void Socket::set_timeout(int recv_ms, int send_ms) {
	struct timeval rt, st;
	memset(&rt, 0, sizeof(rt));
	memset(&st, 0, sizeof(st));
	rt.tv_sec = recv_ms / 1000;
	rt.tv_usec = (recv_ms % 1000) * 1000;

	st.tv_sec = send_ms / 1000;
	st.tv_usec = (send_ms % 1000) * 1000;
	
	int r = setsockopt(_sock, SOL_SOCKET, SO_RCVTIMEO, (char *)&rt, sizeof(rt));
	if (r < 0) 
		throw_net(("setsockopt(SO_RCVTIMEO)"));

	r = setsockopt(_sock, SOL_SOCKET, SO_SNDTIMEO, (char *)&st, sizeof(st));
	if (r < 0) 
		throw_net(("setsockopt(SO_SNDTIMEO)"));
}


void Socket::addr::serialize(Serializator &s) const {
	s.add((unsigned)ip);
	s.add((unsigned)port);
}

void Socket::addr::deserialize(const Serializator &s) {
	unsigned n;
	s.get(n);
	ip = n;
	s.get(n);
	port = n;
//	LOG_DEBUG(("deserialized %08x:%u", ip, port));
}

