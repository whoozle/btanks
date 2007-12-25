#ifndef BTANKS_NET_SCANNER_H__
#define BTANKS_NET_SCANNER_H__

#include <set>
#include <string>
#include "sdlx/thread.h"
#include "sdlx/mutex.h"

class Scanner : public sdlx::Thread {
public:
	Scanner(); 
	~Scanner();
	void scan() { _scan = true; }
	const bool changed() const { return _changed; }

	void get(std::set<std::string> &hosts) const;

private: 
	virtual const int run();
	volatile bool _running, _scan, _changed;
	sdlx::Mutex _hosts_lock;
	std::set<std::string> _hosts;
};


#endif
