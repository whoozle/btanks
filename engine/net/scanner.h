#ifndef BTANKS_NET_SCANNER_H__
#define BTANKS_NET_SCANNER_H__

#include <set>
#include <string>
#include <map>
#include <queue>
#include "sdlx/thread.h"
#include "sdlx/mutex.h"

namespace mrt {
	class Chunk;
	class UDPSocket;
}

class Scanner : public sdlx::Thread {
public:
	struct Host {
		std::string name, map;
		unsigned ping, players, slots;
		
		Host() : name(), map(), ping(0), players(0), slots(0) {}
	};
	typedef std::map<const std::string, Host> HostMap;
	
	Scanner(); 
	~Scanner();
	void scan() { _scan = true; }
	const bool changed() const { return _changed; }
	void reset() { _changed = false; }

	void get(HostMap &hosts) const;

	void add(const std::string &ip, const std::string &name);

private: 
	void createMessage(mrt::Chunk & data);
	void ping(mrt::UDPSocket &udp_sock, unsigned int port);

	virtual const int run();
	volatile bool _running, _scan, _changed;
	sdlx::Mutex _hosts_lock;
	HostMap _hosts;
	typedef std::queue<std::pair<std::string, std::string> > CheckQueue;
	CheckQueue check_queue;
	
	std::string _bindaddr;
	int _port;
};

#endif
