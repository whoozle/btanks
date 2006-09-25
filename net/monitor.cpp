#include "monitor.h"
#include "mrt/chunk.h"
#include "mrt/logger.h"
#include "mrt/socket_set.h"
#include "connection.h"

Monitor::Task::Task(const int id) : id(id), data(new mrt::Chunk), pos(0), len(0) {}
Monitor::Task::Task(const int id, const mrt::Chunk &d) : id(id), data(new mrt::Chunk(d)), pos(0), len(data->getSize()) {}
void Monitor::Task::clear() { delete data; pos = len = 0; }

Monitor::Monitor() : _running(false) {
}

void Monitor::add(const int id, Connection *c) {
	sdlx::AutoMutex m(_connections_mutex);
	_connections[id] = c;
}
	
void Monitor::send(const int id, const mrt::Chunk &data) {
	Task *t = new Task(id, data);
	
	sdlx::AutoMutex m(_connections_mutex);
	_send_q.push_back(t);
}

const bool Monitor::recv(int &id, mrt::Chunk &data) {
	sdlx::AutoMutex m(_result_mutex);
	if (_result_q.empty())
		return false;
	
	id = _result_q.front()->id;
	data = *_result_q.front()->data;
	_result_q.front()->clear();
	
	_result_q.pop_front();
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
		mrt::SocketSet set; 
		for(ConnectionMap::iterator i = _connections.begin(); i != _connections.end(); ++i) {
			set.add(i->second->sock);
		}
		set.check(10);
		
	}
	return 0;
}


Monitor::~Monitor() {
	_running = false;
	wait();

	for(ConnectionMap::iterator i = _connections.begin(); i != _connections.end(); ++i) {
		delete i->second;
	}

	for(TaskQueue::iterator i = _send_q.begin(); i != _send_q.end(); ++i) {
		(*i)->clear();
		delete *i;
	}
	for(TaskQueue::iterator i = _recv_q.begin(); i != _recv_q.end(); ++i) {
		(*i)->clear();
		delete *i;
	}
	for(TaskQueue::iterator i = _result_q.begin(); i != _result_q.end(); ++i) {
		(*i)->clear();
		delete *i;
	}
}
