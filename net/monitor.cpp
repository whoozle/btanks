#include "monitor.h"
#include "mrt/chunk.h"

Monitor::Task::Task() : data(new mrt::Chunk), pos(0), len(0) {}
Monitor::Task::Task(const mrt::Chunk &d) : data(new mrt::Chunk(d)), pos(0), len(data->getSize()) {}
void Monitor::Task::clear() { delete data; pos = len = 0; }

Monitor::Monitor() : _running(false) {
	start();
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

const int Monitor::run() {
	_running = true;
	while(_running) {
		int n;
		{
			sdlx::AutoMutex m(_connections_mutex);
			n = _connections.size();
		}
		if (n == 0)
			SDL_Delay(100);
	}
	return 0;
}


Monitor::~Monitor() {
	_running = false;
	wait();
	for(TaskQueue::iterator i = _send_q.begin(); i != _send_q.end(); ++i) {
		(*i)->clear();
		delete *i;
	}
	for(TaskQueue::iterator i = _recv_q.begin(); i != _recv_q.end(); ++i) {
		(*i)->clear();
		delete *i;
	}
}
