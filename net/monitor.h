#ifndef __BTANKS_NET_MONITOR_H__
#define __BTANKS_NET_MONITOR_H__

/* Battle Tanks Game
 * Copyright (C) 2006-2007 Battle Tanks team
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */


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
	void setCompressionLevel(const int level = 3);
	void add(const int id, Connection *);
	const bool active() const;
	
	void send(const int id, const mrt::Chunk &data);
	void broadcast(const mrt::Chunk &data);
	const bool recv(int &id, mrt::Chunk &data, int &timestamp);
	const bool disconnected(int &id);
	
	void disconnect(const int id);
	
	~Monitor();

private:
	volatile bool _running;
	
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
		unsigned char flags;
		int timestamp;
	};
	Task * createTask(const int id, const mrt::Chunk &data);
	
	typedef std::deque<Task *> TaskQueue;
	TaskQueue _send_q, _recv_q, _result_q;
	std::deque<int> _disconnections;
	
	ConnectionMap _connections;
	sdlx::Mutex _connections_mutex, _result_mutex, _send_q_mutex;
	
	TaskQueue::iterator findTask(TaskQueue &queue, const int conn_id);
	void eraseTask(TaskQueue &q, const TaskQueue::iterator &i);
	void eraseTasks(TaskQueue &q, const int conn_id);
	
	int _comp_level;
};

#endif

