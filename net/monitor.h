#ifndef __BTANKS_NET_MONITOR_H__
#define __BTANKS_NET_MONITOR_H__

#include <deque>
#include "sdlx/thread.h"
#include "sdlx/mutex.h"

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
	
	~Monitor();

private:
	bool _running;
	
	virtual const int run();
	typedef std::deque<Connection *> ConnectionList;
	
	struct Task {
		Task();
		Task(const mrt::Chunk &);
		void clear();
		
		mrt::Chunk *data;
		unsigned int pos;
		unsigned int len;
	};
	
	typedef std::deque<Task *> TaskQueue;
	TaskQueue _send_q, _recv_q;
	
	ConnectionList _connections;
	sdlx::Mutex _connections_mutex;
};

#endif

