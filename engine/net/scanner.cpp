#include "config.h"
#include "scanner.h"
#include "message.h"
#include "mrt/ioexception.h"
#include "mrt/logger.h"
#include "mrt/chunk.h"
#include "mrt/socket_set.h"
#include "mrt/udp_socket.h"
#include "mrt/serializator.h"
#include "mrt/net_exception.h"

#ifdef _WINDOWS
#	include <Winsock2.h>
#else
#	include <sys/socket.h>
#	include <netinet/in.h>
#	include <netinet/ip.h> /* superset of previous */
#endif

Scanner::Scanner() : _running(true), _scan(false), _changed(false) {
	Config->get("multiplayer.bind-address", _bindaddr, std::string());
	Config->get("multiplayer.port", _port, 27255);
	start();
}

Scanner::~Scanner() {
	LOG_DEBUG(("stopping scanner..."));
	_running = false;
	wait();
}

#include "monitor.h" //hack me, move all packet related code to message! 

void Scanner::createMessage(mrt::Chunk &result) {
	mrt::Chunk data;
	Message m(Message::ServerDiscovery);
	Uint32 ticks = SDL_GetTicks();
	mrt::Serializator s;
	s.add(ticks);
	s.finalize(m.data);
	m.serialize2(data);
	Monitor::pack(result, data, 0);
}

const int Scanner::run() {
TRY {

	LOG_DEBUG(("searching for servers at port %d", _port));

	mrt::UDPSocket udp_sock;
	//udp_sock.listen(bindaddr, port, false);
	udp_sock.create();
	udp_sock.setBroadcastMode(1);
	LOG_DEBUG(("udp socket started..."));

	std::set<mrt_uint32_t> banned_addrs;
#ifdef _WINDOWS
	TRY {
		char ac[256];
		if (gethostname(ac, sizeof(ac)) == SOCKET_ERROR) 
			throw_net(("gethostname"));
		
		struct hostent *he = gethostbyname(ac);
		if (he == NULL) 
			throw_net(("gethostbyname"));
		
		for (int i = 0; he->h_addr_list[i] != 0; ++i) {
			struct in_addr addr;
	        memcpy(&addr, he->h_addr_list[i], sizeof(struct in_addr));

			LOG_DEBUG(("my address %s", inet_ntoa(addr)));
			banned_addrs.insert(addr.S_un.S_addr);
		}
	} CATCH("getting host ip addresses", )
	    
#endif	
	
	while(_running) {
		mrt::SocketSet set; 
		set.add(udp_sock, mrt::SocketSet::Exception | mrt::SocketSet::Read);
		if (_scan) {
			mrt::Chunk data;
			createMessage(data);
	
			udp_sock.broadcast(data, _port);	
			_scan = false;
		}

		ping(udp_sock, _port);
	
		if (set.check(100) == 0) {
			continue;
		}
		
		if (set.check(udp_sock, mrt::SocketSet::Exception)) {
			TRY {
				throw_net(("udp_socket"));
			} CATCH("select", )
			LOG_DEBUG(("restarting udp socket..."));
			udp_sock.create();
			udp_sock.setBroadcastMode(1);
		} else if (set.check(udp_sock, mrt::SocketSet::Read)) {
			mrt::Socket::addr addr;
			mrt::Chunk data;
			data.setSize(1500);
			int r = udp_sock.recv(addr, data.getPtr(), data.getSize());
			TRY { 
				if (r == 0 || r == -1)
					throw_net(("udp_sock.recv"));
			} CATCH("recv", continue; );
			data.setSize(r);
			//LOG_DEBUG(("data from addr %s: %s", addr.getAddr().c_str(), data.dump().c_str()));
			TRY {
				Message msg;
				msg.deserialize2(data);
				if (msg.type != Message::ServerDiscovery)
					continue;

				mrt::Serializator s(&msg.data);
				unsigned t0, players, slots;
				s.get(t0);
				s.get(players);
				s.get(slots);
				
				if (players > slots)
					throw_ex(("server returned bogus free slots counter(%u)", players));
				players = slots - players; 
				
				Sint32 delta = (SDL_GetTicks() - t0);
				if (delta < 0) 
					delta = ~delta; //wrapping

				if (delta > 120000) 
					throw_ex(("server returned bogus timestamp value"));
				if (banned_addrs.find(addr.ip) != banned_addrs.end()) {
					LOG_DEBUG(("skipping %s as banned or local", addr.getAddr().c_str()));
					continue;
				}
								
				std::string ip = addr.getAddr();
				LOG_DEBUG(("found server: %s, players: %u, slots: %u", ip.c_str(), players, slots));
				std::string name = addr.getName();
				LOG_DEBUG(("found name: %s", name.c_str()));
				
				sdlx::AutoMutex m(_hosts_lock);
				Host &host = _hosts[ip];
				host.ping = 1 + delta / 2;
				host.name = name;
				host.slots = slots;
				host.players = players;
				host.map = msg.has("map")?msg.get("map"): std::string();
				_changed = true;
			}CATCH("reading message", )
		}
	}
} CATCH("run", return 1;)
	return 0;
}

void Scanner::ping(mrt::UDPSocket &udp_sock, unsigned int port) {
		std::string ip, host;
		{
			sdlx::AutoMutex l(_hosts_lock);
			if (check_queue.empty())
				return;
			ip = check_queue.front().first;
			host = check_queue.front().second;
			check_queue.pop();
		}
		if (ip.empty() && host.empty())
			return;
		
		LOG_DEBUG(("pinging %s/%s", ip.c_str(), host.c_str()));
		TRY {
			mrt::Socket::addr addr;
			addr.port = port;
			if (!host.empty()) {
				addr.getAddr(host);
				if (!addr.empty()) {
					std::string ip = addr.getAddr();
					LOG_DEBUG(("found address %s for %s", ip.c_str(), host.c_str()));
				} else goto check_ip;
			} else {
			check_ip: 
				addr.parse(ip);
				std::string new_host = addr.getName();
				LOG_DEBUG(("found name %s for address %s", new_host.c_str(), ip.c_str()));
				if (!new_host.empty()) {
					host = new_host;
					_changed = true;

					sdlx::AutoMutex l(_hosts_lock);
					_hosts[ip].name = host;
					_hosts[ip].ping = 0;
					_hosts[ip].map.clear();
					_hosts[ip].players = 0;
					_hosts[ip].slots = 0;
				}
			}
			mrt::Chunk data;
			createMessage(data);
			udp_sock.send(addr, data.getPtr(), data.getSize());
		} CATCH("pinging known server", )
}

void Scanner::get(HostMap &hosts) const {
	sdlx::AutoMutex m(_hosts_lock);
	hosts = _hosts;
}

void Scanner::add(const std::string &ip, const std::string &name) {
	sdlx::AutoMutex m(_hosts_lock);
	check_queue.push(CheckQueue::value_type(ip, name));
}

