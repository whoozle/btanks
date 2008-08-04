#ifndef BTANKS_NET_SCANNER_H__
#define BTANKS_NET_SCANNER_H__

#include <set>
#include <string>
#include <map>
#include <queue>
#include "sdlx/thread.h"
#include "sdlx/mutex.h"
#include "mrt/sys_socket.h"
#include "game_type.h"

namespace mrt {
	class Chunk;
	class UDPSocket;
}

class Scanner : public sdlx::Thread {
public:
	struct Host {
		std::string name, map;
		unsigned ping, players, slots;
		GameType game_type;
		
		Host() : name(), map(), ping(0), players(0), slots(0), game_type(GameTypeDeathMatch) {}
	};
	typedef std::map<const mrt::Socket::addr, Host> HostMap;
	
	Scanner(); 
	~Scanner();
	void scan() { _scan = true; }
	const bool changed() const { return _changed; }
	void reset() { _changed = false; }

	void get(HostMap &hosts) const;

	void add(const mrt::Socket::addr &ip, const std::string &name);

private: 

	void createMessage(mrt::Chunk & data);
	void ping(mrt::UDPSocket &udp_sock);

	virtual const int run();
	volatile bool _running, _scan, _changed;
	sdlx::Mutex _hosts_lock;
	HostMap _hosts;
	typedef std::queue<std::pair<mrt::Socket::addr, std::string> > CheckQueue;
	CheckQueue check_queue;
	
	//std::string _bindaddr;
	int _port;


	//dns cache. rewrite all this cryptic scanner stuff asap
	std::string get_name_by_addr(const mrt::Socket::addr &addr);
	mrt::Socket::addr get_addr_by_name(const std::string &name);
	
	typedef std::map<const std::string, mrt::Socket::addr> dns_cache_t;
	dns_cache_t dns_cache;
};

#endif
