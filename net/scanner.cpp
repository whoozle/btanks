#include "config.h"
#include "scanner.h"
#include "message.h"
#include "mrt/ioexception.h"
#include "mrt/logger.h"
#include "mrt/chunk.h"
#include "mrt/socket_set.h"
#include "mrt/udp_socket.h"
#include "mrt/serializator.h"

#ifdef WIN32
#	include <Winsock2.h>
#else
#	include <sys/socket.h>
#	include <netinet/in.h>
#	include <netinet/ip.h> /* superset of previous */
#endif

Scanner::Scanner() : _running(true), _scan(false), _changed(false) {
	start();
}

Scanner::~Scanner() {
	LOG_DEBUG(("stopping scanner..."));
	_running = false;
	wait();
}

const int Scanner::run() {
TRY {
	GET_CONFIG_VALUE("multiplayer.bind-address", std::string, bindaddr, std::string());
	GET_CONFIG_VALUE("multiplayer.port", int, port, 27255);

	LOG_DEBUG(("searching for servers at port %d", port));

	mrt::UDPSocket udp_sock;
	//udp_sock.listen(bindaddr, port, false);
	udp_sock.create();
	udp_sock.setBroadcastMode(1);
	LOG_DEBUG(("udp socket started..."));
	
	
	while(_running) {
		mrt::SocketSet set; 
		set.add(udp_sock, mrt::SocketSet::Exception | mrt::SocketSet::Read);
		if (_scan) {
			Message m(Message::ServerDiscovery);
			Uint32 ticks = SDL_GetTicks();
			mrt::Serializator s;
			s.add(ticks);
			m.data = s.getData();
			
			mrt::Chunk data;
			m.serialize2(data);
	
			udp_sock.broadcast(data, port);	
			_scan = false;
		}

		if (set.check(100) == 0)
			continue;
		
		if (set.check(udp_sock, mrt::SocketSet::Exception)) {
			throw_io(("udp_socket"));
		} else if (set.check(udp_sock, mrt::SocketSet::Read)) {
			mrt::Socket::addr addr;
			mrt::Chunk data;
			data.setSize(1500);
			int r = udp_sock.recv(addr, data.getPtr(), data.getSize());
			if (r == 0 || r == -1)
				throw_io(("udp_sock.read"));
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
					throw_ex(("server returned bogus free slots counter"));
				players = slots - players; 
				
				Sint32 delta = (SDL_GetTicks() - t0);
				if (delta < 0) 
					delta = ~delta; //wrapping

				if (delta > 120000) 
					throw_ex(("server returned bogus timestamp value"));
								
				std::string ip = addr.getAddr();
				LOG_DEBUG(("found server: %s, players: %u, slots: %u", ip.c_str(), players, slots));
				std::string name = addr.getName();
				LOG_DEBUG(("found name: %s", name.c_str()));
				
				sdlx::AutoMutex m(_hosts_lock);
				Host &host = _hosts[ip];
				host.ping = delta / 2;
				host.name = name;
				host.slots = slots;
				host.players = players;
				_changed = true;
			}CATCH("reading message", )
		}
	}
} CATCH("run", return 1;)
	return 0;
}

void Scanner::get(HostMap &hosts) const {
	sdlx::AutoMutex m(_hosts_lock);
	hosts = _hosts;
}
