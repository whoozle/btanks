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

#include "udp_socket.h"
#include "net_exception.h"
#include "mrt/chunk.h"
#include <string.h>

#ifdef _WINDOWS
#	include "Winsock2.h"
#	ifndef socklen_t 
#		define socklen_t int
#	endif

#	ifndef in_addr_t
#		define in_addr_t unsigned long
#	endif
#else
#	include <sys/socket.h>
#	include <netinet/in.h>
#	include <netinet/tcp.h>
#	include <netinet/ip.h> /* superset of previous */
#	include <arpa/inet.h>
#	include <netdb.h>
#endif              


using namespace mrt;

void UDPSocket::listen(const std::string &bindaddr, const unsigned port, const bool reuse) {
	int on = 1;
	if (reuse)
		setsockopt(_sock, SOL_SOCKET, SO_REUSEADDR, (char*)&on, sizeof(on));

	struct sockaddr_in addr;
	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	addr.sin_addr.s_addr = bindaddr.empty()?INADDR_ANY: inet_addr(bindaddr.c_str());
	
	if (bind(_sock, (const sockaddr *)&addr, sizeof(addr)) == -1)
		throw_net(("bind"));
	
//	if (::listen(_sock, 10) == -1)
//		throw_net(("listen"));	
}

void UDPSocket::connect(const mrt::Socket::addr &addr_) {
	struct sockaddr_in addr;
	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(addr_.port);
	addr.sin_addr.s_addr = addr_.ip;
	
	LOG_DEBUG(("connect %s:%u", inet_ntoa(addr.sin_addr), addr_.port));
	if (::connect(_sock, (const struct sockaddr*)&addr, sizeof(addr))	 == -1)
		throw_net(("connect"));
}

void UDPSocket::connect(const std::string &host, const int port) {
	struct sockaddr_in addr;
	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	addr.sin_addr.s_addr = inet_addr(host.c_str());

	if (addr.sin_addr.s_addr == (in_addr_t)-1) {
		//try to resolve host
		hostent *he = gethostbyname(host.c_str());
		if (he == NULL)
			throw_ex(("host '%s' was not found", host.c_str()));
		addr.sin_addr = *(struct in_addr*)(he->h_addr_list[0]);
	}
	
	LOG_DEBUG(("connect %s:%u", inet_ntoa(addr.sin_addr), port));
	if (::connect(_sock, (const struct sockaddr*)&addr, sizeof(addr))	 == -1)
		throw_net(("connect"));
}

UDPSocket::UDPSocket() {
	create();
}

#ifdef _WINDOWS
#	ifndef SIO_UDP_CONNRESET
#		define SIO_UDP_CONNRESET _WSAIOW(IOC_VENDOR, 12)
#	endif
#endif


void UDPSocket::create() {
	Socket::create(PF_INET, SOCK_DGRAM, 0);

#ifdef _WINDOWS
	DWORD dwBytesReturned = 0;
	BOOL bNewBehavior = FALSE;
	DWORD status;

	// disable  new behavior using
	// IOCTL: SIO_UDP_CONNRESET
	status = WSAIoctl(_sock, SIO_UDP_CONNRESET, &bNewBehavior, sizeof(bNewBehavior), NULL, 0, &dwBytesReturned, NULL, NULL);

#endif	
}

const int UDPSocket::send(const Socket::addr &addr, const void *data, const int len) const {
	sockaddr_in sockaddr;

	memset(&sockaddr, 0, sizeof(sockaddr));

	sockaddr.sin_family = AF_INET;
	sockaddr.sin_addr.s_addr = addr.ip;
	sockaddr.sin_port = htons(addr.port);
	
	return ::sendto(_sock, (const char *)data, len, 0, (struct sockaddr *)&sockaddr, sizeof(sockaddr));
}

const int UDPSocket::recv(Socket::addr &addr, void *data, const int len) const {
	sockaddr_in sockaddr;
	socklen_t socklen = sizeof(sockaddr);
	memset(&sockaddr, 0, sizeof(sockaddr));

	sockaddr.sin_family = AF_INET;

	int r = ::recvfrom(_sock, (char *)data, len, 0, (struct sockaddr *)&sockaddr, &socklen);
	addr.ip = sockaddr.sin_addr.s_addr;
	addr.port = ntohs(sockaddr.sin_port);
	return r;
}

void UDPSocket::set_broadcast_mode(int val) {
	if (_sock == -1)
		throw_ex(("setBroadcast called on uninitialized socket"));
	TRY { 
		int r = setsockopt(_sock, SOL_SOCKET, SO_BROADCAST, (char *)&val, sizeof(val)); //win32 hack
		if (r == -1)
			throw_net(("setsockopt"));
	} CATCH("setsockopt(IPPROTO_UDP, SO_BROADCAST)", {});
}

#ifdef _WINDOWS
void UDPSocket::broadcast(const mrt::Chunk &data, const int port) {
	TRY {
		LOG_DEBUG(("broadcasting packet[%u]", (unsigned)data.get_size()));
		if (send(mrt::Socket::addr(INADDR_BROADCAST, port), data.get_ptr(), data.get_size()) == -1)
			throw_net(("sendto"));
	} CATCH("broadcast", );
}
#else 
#	include <net/if.h>
#	include <ifaddrs.h>

void UDPSocket::broadcast(const mrt::Chunk &data, const int port) {
	LOG_DEBUG(("broadcasting packet[%u]", (unsigned)data.get_size()));
	struct ifaddrs * ifs = NULL;
	TRY {
		if (getifaddrs(&ifs) == -1)
			throw_net(("getifaddrs"));
		
		for(struct ifaddrs * i = ifs; i->ifa_next != NULL; i = i->ifa_next) {
			int flags = i->ifa_flags;
			if (!(flags & IFF_BROADCAST) || !(flags & IFF_UP) || flags & IFF_LOOPBACK)
				 continue;
#		ifdef __linux__	
			if (i->ifa_ifu.ifu_broadaddr == NULL || i->ifa_ifu.ifu_broadaddr->sa_family != PF_INET)
				continue;
			
			sockaddr_in *addr = (sockaddr_in *)i->ifa_ifu.ifu_broadaddr;
			LOG_DEBUG(("interface: %s, ifu_broadaddr: %s", i->ifa_name, inet_ntoa(addr->sin_addr)));
			TRY {
				if (send(mrt::Socket::addr(addr->sin_addr.s_addr, port), data.get_ptr(), data.get_size()) == -1)
					throw_net(("sendto"));
			} CATCH("sendto", );
#		else
			LOG_WARN(("implement broadcast address obtaining."));
#		endif
		}
	} CATCH("broadcast", );
	if (ifs != NULL) 	
		freeifaddrs(ifs);
}

#endif
