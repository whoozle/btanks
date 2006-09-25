#ifndef __BTANKS_NET_MONITOR_H__
#define __BTANKS_NET_MONITOR_H__

#include <deque>
#include <map>
#include "sdlx/thread.h"
#include "sdlx/mutex.h"

namespace mrt {
	class Chunk;
}

class Connection;
class Monitor : public sdlx::Thread {
public:
	Monitor();
	void add(const int id, Connection *);
	
	void send(const int id, const mrt::Chunk &data);
	const bool recv(int &id, mrt::Chunk &data);
	
	~Monitor();

private:
	bool _running;
	
	virtual const int run();
	typedef std::map<const int, Connection *> ConnectionMap;
	
	struct Task {
		Task(const int id);
		Task(const int id, const mrt::Chunk &);
		Task(const int id, const int size);
		void clear();
		
		int id;
		mrt::Chunk *data;
		unsigned int pos;
		unsigned int len;
		bool size_task;
	};
	
	typedef std::deque<Task *> TaskQueue;
	TaskQueue _send_q, _recv_q, _result_q;
	
	ConnectionMap _connections;
	sdlx::Mutex _connections_mutex, _result_mutex;
	
	TaskQueue::iterator findTask(TaskQueue &queue, const int conn_id);
	void eraseTask(TaskQueue &q, const TaskQueue::iterator &i);
	void eraseTasks(TaskQueue &q, const int conn_id);
};

#endif

