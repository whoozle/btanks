#ifndef BTANKS_NET_SCANNER_H__
#define BTANKS_NET_SCANNER_H__

#include <set>
#include <string>
#include <map>
#include "sdlx/thread.h"
#include "sdlx/mutex.h"

class Scanner : public sdlx::Thread {
public:
	struct Host {
		std::string name;
		int ping;
	};
	typedef std::map<const std::string, Host> HostMap;
	
	Scanner(); 
	~Scanner();
	void scan() { _scan = true; }
	const bool changed() const { return _changed; }
	void reset() { _changed = false; }

	void get(HostMap &hosts) const;

private: 
	virtual const int run();
	volatile bool _running, _scan, _changed;
	sdlx::Mutex _hosts_lock;
	HostMap _hosts;
};


#endif
