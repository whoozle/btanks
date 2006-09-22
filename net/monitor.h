#ifndef __BTANKS_NET_MONITOR_H__
#define __BTANKS_NET_MONITOR_H__

#include "sdlx/thread.h"
#include <deque>

namespace mrt {
	class Chunk;
}

class Connection;
class Monitor : public sdlx::Thread {
public:
	Monitor();
	void add(Connection *);
	
	virtual void onRecv(const mrt::Chunk &data) = 0;
	void send(const mrt::Chunk &data);

private:
	virtual const int run();
	typedef std::deque<Connection *> ConnectionList;
	
	struct Task {
		Task();
		Task(const mrt::Chunk &);
		
		mrt::Chunk *data;
		unsigned int pos;
		unsigned int len;
	};
	
	ConnectionList _connections;
};

#endif

