#include "monitor.h"
#include "mrt/chunk.h"
#include "mrt/logger.h"
#include "sdlx/socket_set.h"
#include "connection.h"

Monitor::Task::Task() : data(new mrt::Chunk), pos(0), len(0) {}
Monitor::Task::Task(const mrt::Chunk &d) : data(new mrt::Chunk(d)), pos(0), len(data->getSize()) {}
void Monitor::Task::clear() { delete data; pos = len = 0; }

Monitor::Monitor() : _running(false) {
}

void Monitor::add(Connection *c) {
	sdlx::AutoMutex m(_connections_mutex);
	_connections.push_back(c);
}
	
void Monitor::send(const mrt::Chunk &data) {
	Task *t = new Task(data);
	
	sdlx::AutoMutex m(_connections_mutex);
	_send_q.push_back(t);
}

const bool Monitor::recv(mrt::Chunk &data) {
	sdlx::AutoMutex m(_result_mutex);
	if (_result.empty())
		return false;
	
	data = *_result.front();
	_result.pop_front();
	return true;
}


const int Monitor::run() {
	_running = true;
	LOG_DEBUG(("network monitor thread was started..."));
	while(_running) {
		int n;
		{
			sdlx::AutoMutex m(_connections_mutex);
			n = _connections.size();
		}
		if (n == 0) {
			SDL_Delay(100);
			continue;
		} 
		for(TaskQueue::iterator i = _send_q.begin(); i != _send_q.end(); ++i) {
			Task *t = *i;
		}
		sdlx::SocketSet set(n); 
		for(ConnectionList::iterator i = _connections.begin(); i != _connections.end(); ++i) {
			set.add(*(*i)->sock);
		}
		set.check(10);
		
	}
	return 0;
}


Monitor::~Monitor() {
	_running = false;
	wait();

	for(ConnectionList::iterator i = _connections.begin(); i != _connections.end(); ++i) {
		delete *i;
	}

	for(TaskQueue::iterator i = _send_q.begin(); i != _send_q.end(); ++i) {
		(*i)->clear();
		delete *i;
	}
	for(TaskQueue::iterator i = _recv_q.begin(); i != _recv_q.end(); ++i) {
		(*i)->clear();
		delete *i;
	}
	for(ResultQueue::iterator i = _result.begin(); i != _result.end(); ++i) {
		delete *i;
	}
}
