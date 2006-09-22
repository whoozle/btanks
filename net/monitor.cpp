#include "monitor.h"
#include "mrt/chunk.h"

Monitor::Task::Task() : data(new mrt::Chunk), pos(0), len(0) {}
Monitor::Task::Task(const mrt::Chunk &d) : data(new mrt::Chunk(d)), pos(0), len(data->getSize()) {}
Monitor::Monitor() {}

void Monitor::add(Connection *c) {
	_connections.push_back(c);
}
	
void Monitor::send(const mrt::Chunk &data) {

}

const int Monitor::run() {
	return 0;
}
