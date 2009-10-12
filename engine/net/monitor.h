#ifndef __BTANKS_NET_MONITOR_H__
#define __BTANKS_NET_MONITOR_H__

/* Battle Tanks Game
 * Copyright (C) 2006-2009 Battle Tanks team
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

/* 
 * Additional rights can be granted beyond the GNU General Public License 
 * on the terms provided in the Exception. If you modify this file, 
 * you may extend this exception to your version of the file, 
 * but you are not obligated to do so. If you do not wish to provide this
 * exception without modification, you must delete this exception statement
 * from your version and license this file solely under the GPL without exception. 
*/


#include <list>
#include <map>
#include <string>
#include "sdlx/thread.h"
#include "sdlx/mutex.h"
#include "mrt/sys_socket.h"
#include "mrt/chunk.h"

namespace mrt {
	class Chunk;
	class UDPSocket;
	class TCPSocket;
}

class Connection;
class Monitor : public sdlx::Thread {
public:
	Monitor(const int cl);
	void setCompressionLevel(const int level = 3);
	void add(const int id, Connection *);
	const bool active() const;
	
	void connect(const mrt::Socket::addr &host);
	void send(const int id, const mrt::Chunk &data, const bool dgram = false);
	void broadcast(const mrt::Chunk &data, const bool dgram = false);
	void accept();
	const bool recv(int &id, mrt::Chunk &data);
	const bool disconnected(int &id);
	
	void disconnect(const int id);
	
	bool connected(int id) const;
	
	void add(mrt::UDPSocket *socket);
	void add(mrt::TCPSocket *server_socket);
	
	~Monitor();
	Connection *pop();

	static void pack(mrt::Chunk &result, const mrt::Chunk &rawdata, const int comp_level);
	static void parse(mrt::Chunk &data, const unsigned char *buf, const int len);

private:
	void _accept();
	void _connect();
	volatile bool _running;
	
	virtual const int run();
	typedef std::map<const int, Connection *> ConnectionMap;
	
	struct Task {
		Task(const int id);
		Task(const int id, const mrt::Chunk &);
		Task(const int id, const int size);
		~Task();
		
		int id;
		mrt::Chunk data;
		unsigned int pos;
		unsigned int len;
		bool size_task;
		unsigned char flags;
	private: 
		Task(const Task &);
		const Task& operator=(const Task &);
	};
	
	Task * createTask(const int id, const mrt::Chunk &data);
	
	typedef std::list<Task *> TaskQueue;
	TaskQueue _send_q, _send_dgram, _recv_q, _result_q, _result_q_dgram;
	std::list<mrt::TCPSocket *> _new_connections;
	std::list<int> _disconnections;
	
	ConnectionMap _connections;
	sdlx::Mutex _connections_mutex, _result_mutex, _result_dgram_mutex, _send_q_mutex, _send_dgram_mutex;
	
	TaskQueue::iterator findTask(TaskQueue &queue, const int conn_id);
	void eraseTask(TaskQueue &q, const TaskQueue::iterator &i);
	void eraseTasks(TaskQueue &q, const int conn_id);
	
	int _comp_level;
	mrt::UDPSocket *_dgram_sock;
	mrt::TCPSocket *_server_sock;
	mrt::Socket::addr _connect_host;
};

#endif

