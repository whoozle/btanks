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
#include "mrt/tcp_socket.h"

#ifdef _WINDOWS
#	include <Winsock2.h>
#else
#	include <sys/socket.h>
#	include <netinet/in.h>
#	include <netinet/ip.h> /* superset of previous */
#	include <arpa/inet.h>
#endif

Scanner::Scanner() : _running(true), _scan(false), _changed(false) {
	//Config->get("multiplayer.bind-address", _bindaddr, std::string());
	Config->get("multiplayer.port", _port, 27255);
	start();
}

Scanner::~Scanner() {
	LOG_DEBUG(("stopping scanner..."));
	_running = false;
	sdlx::Thread::kill();
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
	udp_sock.set_broadcast_mode(1);
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
	} CATCH("getting host ip addresses", {})
	    
#endif	
	
	while(_running) {
		mrt::SocketSet set; 
		set.add(udp_sock, mrt::SocketSet::Exception | mrt::SocketSet::Read);
		if (_scan) {
			mrt::Chunk data;
			TRY {
				GET_CONFIG_VALUE("multiplayer.client.master-server", std::string, mname, "btanks.servegame.com");
				GET_CONFIG_VALUE("multiplayer.client.master-server-port", int, mport, 27254);
				LOG_DEBUG(("connecting to the master server %s:%d...", mname.c_str(), mport));
				mrt::TCPSocket sock;
				sock.set_timeout(3000, 3000);
				sock.connect(mname, mport);
				char req = 'c';
				sock.send(&req, 1);

				mrt::Chunk data;
				char buf[1500];
				int r;
				while((r = sock.recv(buf, sizeof(buf))) > 0) {
					data.append(buf, r);
				}
				if (!_running)
					break;
				LOG_DEBUG(("got %u bytes.", (unsigned)data.get_size()));
				mrt::Serializator s(&data);
				while(!s.end()) {
					mrt::Socket::addr addr;
					s.get(addr);
					LOG_DEBUG(("got %s from master server", addr.getAddr().c_str()));
				
					sdlx::AutoMutex m(_hosts_lock);
					check_queue.push(CheckQueue::value_type(addr, std::string()));
				}
			} CATCH("scanning", {});

			createMessage(data);
	
			udp_sock.broadcast(data, _port);	
			_scan = false;
		}

		ping(udp_sock);
	
		if (set.check(100) == 0) {
			continue;
		}
		
		if (set.check(udp_sock, mrt::SocketSet::Exception)) {
			TRY {
				throw_net(("udp_socket"));
			} CATCH("select", {})
			LOG_DEBUG(("restarting udp socket..."));
			udp_sock.create();
			udp_sock.set_broadcast_mode(1);
		} else if (set.check(udp_sock, mrt::SocketSet::Read)) {
			mrt::Socket::addr addr;
			unsigned char buf[1500]; //fixme ?
			int r = udp_sock.recv(addr, buf, sizeof(buf));
			TRY { 
				if (r == 0 || r == -1)
					throw_net(("udp_sock.recv"));
			} CATCH("recv", continue; );

			LOG_DEBUG(("data from addr %s (%d)", addr.getAddr().c_str(), r));
			TRY {
				mrt::Chunk data;
				Monitor::parse(data, buf, r);
				
				Message msg;
				msg.deserialize2(data);
				if (msg.type != Message::ServerDiscovery)
					continue;

				mrt::Serializator s(&msg.data);
				unsigned t0, players, slots;
				s.get(t0);
				s.get(players);
				s.get(slots);
				int game_type;
				s.get(game_type);
				if (game_type < GameTypeDeathMatch || game_type > GameTypeTeamDeathMatch) 
					throw_ex(("got invalid game_type: %d", game_type));
				
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
								
				std::string ip = addr.getAddr(false);
				LOG_DEBUG(("found server: %s, players: %u, slots: %u", ip.c_str(), players, slots));
				std::string name = get_name_by_addr(addr);
				if (name == "netive.ru" || name == "www.nips.ru" || name == "nips.ru") {
					name = "btanks.media.netive.ru";
				}
				LOG_DEBUG(("found name: %s", name.c_str()));
				
				sdlx::AutoMutex m(_hosts_lock);
				Host &host = _hosts[addr];
				host.ping = 1 + delta / 2;
				host.name = name;
				host.slots = slots;
				host.players = players;
				host.map = msg.has("map")?msg.get("map"): std::string();
				host.game_type = (GameType) game_type;
				_changed = true;
			}CATCH("reading message", {})
		}
	}
	return 0;
} CATCH("run", return 1)
}

void Scanner::ping(mrt::UDPSocket &udp_sock) {
		mrt::Socket::addr addr;
		std::string host;
		{
			sdlx::AutoMutex l(_hosts_lock);
			if (check_queue.empty())
				return;
			addr = check_queue.front().first;
			host = check_queue.front().second;
			check_queue.pop();
		}
		if (addr.ip == 0 && host.empty())
			return;
		
		LOG_DEBUG(("pinging %s/%s", addr.getAddr().c_str(), host.c_str()));
		TRY {
			if (!host.empty()) {
				int port = addr.port;
				addr = get_addr_by_name(host);
				addr.port = port;
				if (!addr.empty()) {
					std::string ip = addr.getAddr();
					LOG_DEBUG(("found address %s for %s", ip.c_str(), host.c_str()));
				} else goto check_ip;
			} else {
			check_ip: 
				std::string new_host = get_name_by_addr(addr);
				if (new_host == "netive.ru" || new_host == "www.nips.ru" || new_host == "nips.ru") {
					new_host = "btanks.media.netive.ru";
				}
				LOG_DEBUG(("found name %s for address %s", new_host.c_str(), addr.getAddr().c_str()));
				if (!new_host.empty()) {
					host = new_host;
					_changed = true;

					sdlx::AutoMutex l(_hosts_lock);
					Host &h = _hosts[addr];
					h.name = host;
					h.ping = 0;
					h.map.clear();
					h.players = 0;
					h.slots = 0;
				}
			}
			mrt::Chunk data;
			createMessage(data);
			udp_sock.send(addr, data.get_ptr(), data.get_size());
		} CATCH("pinging known server", {} )
}

void Scanner::get(HostMap &hosts) const {
	sdlx::AutoMutex m(_hosts_lock);
	hosts = _hosts;
}

void Scanner::add(const mrt::Socket::addr &addr_, const std::string &name) {
	sdlx::AutoMutex m(_hosts_lock);
	mrt::Socket::addr addr = addr_;
	if (addr.port == 0)
		addr.port = _port;
	check_queue.push(CheckQueue::value_type(addr, name));
}

std::string Scanner::get_name_by_addr(const mrt::Socket::addr &addr) {
	for(dns_cache_t::const_iterator i = dns_cache.begin(); i != dns_cache.end(); ++i) {
		if (i->second.ip == addr.ip) 
			return i->first;
	}
	std::string name = addr.getName();
	dns_cache.insert(dns_cache_t::value_type(name, addr));
	return name;
}

mrt::Socket::addr Scanner::get_addr_by_name(const std::string &name) {
	dns_cache_t::const_iterator i = dns_cache.find(name);
	if (i != dns_cache.end()) 
		return i->second;
	mrt::Socket::addr addr; 
	addr.getAddrByName(name);
	dns_cache.insert(dns_cache_t::value_type(name, addr));
	return addr;
}
