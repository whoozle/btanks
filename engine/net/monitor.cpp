
/* Battle Tanks Game
 * Copyright (C) 2006-2008 Battle Tanks team
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

#include "monitor.h"
#include "mrt/chunk.h"
#include "mrt/logger.h"
#include "mrt/exception.h"
#include "mrt/socket_set.h"
#include "mrt/tcp_socket.h"
#include "mrt/udp_socket.h"
#include "mrt/gzip.h"
#include "connection.h"
#include "sdlx/timer.h"
#include "message.h"
#include "player_manager.h"

#ifdef _WINDOWS
#	include "Winsock2.h"
#else
#	include <sys/socket.h>
#	include <netinet/in.h>
#	include <netinet/ip.h> /* superset of previous */
#	include <arpa/inet.h>
#	include <netdb.h>
#endif              

#include <set>
#ifdef _WINDOWS
	typedef unsigned __int32 uint32_t;
#else
#	include <stdint.h>
#endif


void Monitor::connect(const std::string &host) {
	sdlx::AutoMutex m(_connections_mutex);
	_connect_host = host;
}

void Monitor::_connect() {
	std::string host;
	{
		sdlx::AutoMutex m(_connections_mutex);
		host = _connect_host;
		_connect_host.clear();
	}
	LOG_DEBUG(("[monitor thread] connecting to %s:%d", host.c_str(), _port));
	
	_dgram_sock->connect(host, _port);

	Connection *conn = NULL;
	TRY { 
		conn = new Connection(new mrt::TCPSocket);
		conn->sock->connect(host, _port, true);
		conn->sock->noDelay();
		add(0, conn);
		conn = NULL;
	} CATCH("init", {delete conn; conn = NULL; throw; });
}

//public accept

void Monitor::accept() {
	{
		sdlx::AutoMutex m(_connections_mutex);
		if (_new_connections.empty())
			return;
	}

	LOG_DEBUG(("client(s) connected... [main thread context]"));
	Message msg(Message::ServerStatus);
	int id = PlayerManager->onConnect(msg);

	{
		sdlx::AutoMutex m(_connections_mutex);
		delete _connections[id];
		_connections[id] = new Connection(_new_connections.front());
		_new_connections.pop_front();
	}

	mrt::Chunk data;
	msg.serialize2(data);

	LOG_DEBUG(("sending message '%s' to %d", msg.getType(), id));
	send(id, data, msg.realtime());
}

//private accept
void Monitor::_accept() {
	mrt::TCPSocket *s = NULL;
	TRY {
		s = new mrt::TCPSocket;
		_server_sock->accept(*s);
		s->noDelay();
		sdlx::AutoMutex m(_connections_mutex);
		_new_connections.push_back(s);
	} CATCH("accept", { delete s; s = NULL; })
}

Monitor::Task::Task(const int id) : 
	id(id), data(new mrt::Chunk), pos(0), len(0), size_task(false), flags(0), timestamp(0) {}

Monitor::Task::Task(const int id, const mrt::Chunk &d) : 
	id(id), data(new mrt::Chunk(d)), pos(0), len(data->getSize()), size_task(false), flags(0), timestamp(0) {}

Monitor::Task::Task(const int id, const int size) : 
	id(id), data(new mrt::Chunk(size)), pos(0), len(data->getSize()), size_task(false), flags(0), timestamp(0) {}

void Monitor::Task::clear() { delete data; pos = len = 0; }

Monitor::Monitor(const int port, const int cl) : 
	_port(port), _running(false), 
	_send_q(), _recv_q(), _result_q(), 
	_disconnections(), _connections(), 
	_connections_mutex(), _result_mutex(), _send_q_mutex(), 
	_comp_level(0), _dgram_sock(NULL), _server_sock(NULL) {
	_comp_level = cl;
	LOG_DEBUG(("compression level = %d", _comp_level));
}

void Monitor::add(mrt::UDPSocket *socket) {
	_dgram_sock = socket;
}

void Monitor::add(mrt::TCPSocket *socket) {
	_server_sock = socket;
}

void Monitor::add(const int id, Connection *c) {
	sdlx::AutoMutex m(_connections_mutex);
	delete _connections[id];
	_connections[id] = c;
}

const bool Monitor::active() const {
	sdlx::AutoMutex m(_connections_mutex);
	return !_connections.empty();
}


Monitor::Task * Monitor::createTask(const int id, const mrt::Chunk &rawdata) {
	mrt::Chunk data;
	unsigned char flags = 0;
	if (_comp_level > 0) {
		flags = 1; //compressed
		mrt::ZStream::compress(data, rawdata, false, _comp_level);
		LOG_DEBUG(("sending(%d, %u) (compressed: %u)", id, (unsigned)rawdata.getSize(), (unsigned)data.getSize()));
	} else data = rawdata; //fixme: optimize it somehow.

	int size = data.getSize();

	Task *t = new Task(id, size + 9);

	char * ptr = (char *) t->data->getPtr();

	uint32_t nsize = htonl((long)size);
	memcpy(ptr, &nsize, 4);

	nsize = htonl((long)SDL_GetTicks());
	memcpy(ptr + 4, &nsize, 4);

	*((unsigned char *)t->data->getPtr() + 8) = flags;
	memcpy((unsigned char *)t->data->getPtr() + 9, data.getPtr(), size);
	
	return t;
}

void Monitor::pack(mrt::Chunk &result, const mrt::Chunk &rawdata, const int comp_level) {
	mrt::Chunk data;
	unsigned char flags = 0;
	if (comp_level > 0) {
		flags = 1; //compressed
		mrt::ZStream::compress(data, rawdata, false, comp_level);
	} else data = rawdata; //fixme: optimize it somehow.

	int size = data.getSize();

	result.setSize(size + 9);

	unsigned char * ptr = static_cast<unsigned char *>(result.getPtr());

	uint32_t nsize = htonl((long)size);
	memcpy(ptr, &nsize, 4);

	nsize = htonl((long)SDL_GetTicks());
	memcpy(ptr + 4, &nsize, 4);

	*(ptr + 8) = flags;
	memcpy(ptr + 9, data.getPtr(), size);
}

void Monitor::send(const int id, const mrt::Chunk &rawdata, const bool dgram) {
	{
		sdlx::AutoMutex m(_connections_mutex);
		if (_connections.find(id) == _connections.end()) {
			throw_ex(("sending data to non-existent connection %d", id));
			return;
		}
	}
	Task *t = createTask(id, rawdata);
	
	sdlx::AutoMutex m(dgram?_send_dgram_mutex:_send_q_mutex);
	(dgram?_send_dgram:_send_q).push_back(t);
}

void Monitor::broadcast(const mrt::Chunk &data, const bool dgram) {
	std::queue<Task *> tasks;
	{
		sdlx::AutoMutex m(_connections_mutex);
		for(ConnectionMap::const_iterator i = _connections.begin(); i != _connections.end(); ++i) {
			tasks.push(createTask(i->first, data));	
		}
	}
	
	sdlx::AutoMutex m(dgram?_send_dgram_mutex:_send_q_mutex);
	while(!tasks.empty()) {
		(dgram?_send_dgram:_send_q).push_back(tasks.front());
		tasks.pop();
	}
}


Monitor::TaskQueue::iterator Monitor::findTask(TaskQueue &queue, const int conn_id) {
	Monitor::TaskQueue::iterator i;
	for(i = queue.begin(); i != queue.end(); ++i) 
		if ((*i)->id == conn_id)
			return i;
	return i;
}


const bool Monitor::recv(int &id, mrt::Chunk &data, int &timestamp) {
	sdlx::AutoMutex m(_result_mutex);
	if (_result_q.empty())
		return false;
	
	Task *task = _result_q.front();
	_result_q.pop_front();
	m.unlock();
	TRY { 
		id = task->id;
		data = *(task->data);
		timestamp = task->timestamp;

		task->clear();
	} CATCH("recv", { task->clear(); delete task; throw; });
	delete task;
	return true;
}

const bool Monitor::disconnected(int &id) {
	sdlx::AutoMutex m(_result_mutex);
	if (_disconnections.empty())
		return false;
	id = _disconnections.front();
	_disconnections.pop_front();
	return true;
}


void Monitor::eraseTask(TaskQueue &q, const TaskQueue::iterator &i) {
	(*i)->clear();
	delete *i;
	q.erase(i);
}

void Monitor::eraseTasks(TaskQueue &q, const int conn_id) {
	for(TaskQueue::iterator i = q.begin(); i != q.end(); ) {
		if ((*i)->id == conn_id) {
			delete *i;
			i = q.erase(i);
		} else ++i;
	}
}


void Monitor::disconnect(const int cid) {
	LOG_DEBUG(("disconnecting client %d.", cid));
	{ 
		sdlx::AutoMutex m(_connections_mutex); 
		_connections.erase(cid);
	}
	
	{ 
		sdlx::AutoMutex m(_send_q_mutex); 
		eraseTasks(_send_q, cid);
	}
				
	{
		sdlx::AutoMutex m(_result_mutex);
		_disconnections.push_back(cid);
	}
}

void Monitor::parse(mrt::Chunk &data, const unsigned char *buf, const int r, int &ts) {
	if (r < 9) 
		throw_ex(("packet too short (%u)", (unsigned)r));
	
	unsigned long len = ntohl(*((const uint32_t *)buf));
	ts = ntohl(*((const uint32_t *)(buf + 4)));
	if (len > 1048576)
		throw_ex(("recv'ed packet length of %u. it seems to be far too long for regular packet (probably broken/obsoleted client)", (unsigned int)len));

	unsigned char flags = buf[8];

	if (flags & 1) {
		mrt::Chunk src;
		src.setData(buf + 9, r - 9);
		mrt::ZStream::decompress(data, src, false);
	} else {
		data.setData(buf + 9, r - 9);
	}
}

#include "message.h"
#include "player_manager.h" //fixme
#include "tmx/map.h"

const int Monitor::run() {
TRY {
	_running = true;
	LOG_DEBUG(("network monitor thread was started..."));
	{
		sdlx::AutoMutex m(_connections_mutex);
		if (!_connect_host.empty())
			_connect();
	}
	while(_running) {
		std::set<int> cids;
		mrt::SocketSet set; 
		{
			sdlx::AutoMutex m(_connections_mutex);
			for(ConnectionMap::iterator i = _connections.begin(); i != _connections.end(); ++i) {
				cids.insert(i->first);
				int how = mrt::SocketSet::Read | mrt::SocketSet::Exception;
				if (findTask(_send_q, i->first) != _send_q.end()) 
					how |= mrt::SocketSet::Write;
			
				set.add(i->second->sock, how);
			}
		}

		if (cids.empty() && _dgram_sock == NULL) {
			sdlx::Timer::microsleep("waiting for connection", 10000);
			continue;
		}
		
		if (_dgram_sock != NULL) {
			sdlx::AutoMutex m(_send_dgram_mutex);
			set.add(_dgram_sock, _send_dgram.empty()? mrt::SocketSet::Read: mrt::SocketSet::Read | mrt::SocketSet::Write);
		}

		if (_server_sock != NULL) {
			set.add(_server_sock, mrt::SocketSet::Read);
		}

		if (set.check(1) == 0) {
			///LOG_DEBUG(("no events"));
			continue;
		}

		if (_server_sock != NULL && set.check(_server_sock, mrt::SocketSet::Read)) {
			LOG_DEBUG(("accepting connection..."));
			_accept();
		}
			
		if (_dgram_sock != NULL && set.check(_dgram_sock, mrt::SocketSet::Read)) {
			unsigned char buf[1500]; //fixme
			mrt::Socket::addr addr;
			int r = _dgram_sock->recv(addr, buf, sizeof(buf));
			//LOG_DEBUG(("recv() == %d", r));
			//if (r > 9) {
			TRY {
				sdlx::AutoMutex m(_connections_mutex);
				ConnectionMap::iterator i;
				for(i = _connections.begin(); i != _connections.end(); ++i) {
					//fixme: translate remote udp socket to connection id ! 
					if (addr.ip == i->second->sock->getAddress().ip) 
						break;
				}
				if (i != _connections.end()) {
					//update udp connection status. 
					i->second->addr = addr;
				
					m.unlock();
	
					Task * t = NULL;
					TRY {
						t = new Task(i->first);
					
						parse(*t->data, buf, r, t->timestamp);
					} CATCH("processing datagram", {
						delete t;
						t = NULL;
						throw;
					});
					t->len = t->data->getSize();
					
					//LOG_DEBUG(("recv(%d, %u)", t->id, (unsigned)t->data->getSize()));

					sdlx::AutoMutex m2(_result_mutex);
					_result_q.push_back(t);
				} else {
					bool ok = false;

					TRY {
						mrt::Chunk data;
						int ts = 0;
						parse(data, buf, r, ts);

						Message msg;
						msg.deserialize2(data);
						if (msg.type != Message::ServerDiscovery) 
							throw_ex(("wrong message type: %s", msg.getType()));

						ok = true;
						LOG_DEBUG(("server discovery datagram from the %s", addr.getAddr().c_str()));
						unsigned t0;
						{
							mrt::Serializator in(&msg.data);
							in.get(t0);
						}
						mrt::Serializator out;
							
						out.add(t0);
						out.add((unsigned)PlayerManager->getFreeSlotsCount());
						out.add((unsigned)PlayerManager->getSlotsCount());
						out.finalize(msg.data);
						msg.set("map", Map->getName());
							
						msg.serialize2(data);
						mrt::Chunk result;
						pack(result, data, _comp_level);
						_dgram_sock->send(addr, result.getPtr(), result.getSize()); //returning same message
					} CATCH("discovery message", );
					if (!ok) {
						LOG_WARN(("incoming datagram from unknown client(%s:%d)", addr.getAddr().c_str(), addr.port));
					}
				}
			} CATCH("datagram", )
			/*} else {
				LOG_WARN(("short datagram recv-ed [%d]", r));
			}*/
		}
		
		if (_dgram_sock != NULL && set.check(_dgram_sock, mrt::SocketSet::Write)) {
			Task *task = NULL;
			{
				sdlx::AutoMutex m(_send_dgram_mutex);
				if (!_send_dgram.empty()) {
					task = _send_dgram.front();
					_send_dgram.pop_front();
				} else LOG_WARN(("no event in datagram write queue!"));
			}
			if (task != NULL) {
				sdlx::AutoMutex m(_connections_mutex);
				ConnectionMap::const_iterator i = _connections.find(task->id);
				if (i != _connections.end()) {
					mrt::Socket::addr addr= i->second->addr.empty()?i->second->sock->getAddress():i->second->addr;
					int r = _dgram_sock->send(addr, task->data->getPtr(), task->data->getSize());
					if (r != (int)task->data->getSize()) {
						LOG_WARN(("short sendto(%08x:%d, %u) == %d", addr.ip, addr.port, (unsigned)task->data->getSize(), r));
					}
				} else LOG_WARN(("task to invalid connection %d found (purged)", task->id));
				task->clear();
				delete task;
			}
		}
		
		for(std::set<int>::iterator i = cids.begin(); i != cids.end(); ++i) {
			const int cid = *i;
			const mrt::TCPSocket *sock = NULL;
			{
				sdlx::AutoMutex m(_connections_mutex);
				ConnectionMap::const_iterator i = _connections.find(cid);
				if (i == _connections.end())
					continue;
				sock = i->second->sock;
			}
			
			if (set.check(sock, mrt::SocketSet::Exception)) {
				//fixme: notify upper layer 
			disconnect: 
				disconnect(cid);
				continue;
			}

			if (set.check(sock, mrt::SocketSet::Read)) {

				TaskQueue::iterator ti = findTask(_recv_q, cid);
				if (ti == _recv_q.end()) {
					Task *t = new Task(cid, 9);
					t->size_task = true;
					_recv_q.push_back(t);
					//LOG_DEBUG(("added size task to r-queue"));
					ti = findTask(_recv_q, cid);
					assert(ti != _recv_q.end());
				}
				Task *t = *ti;
			
				int estimate = t->len - t->pos;
				assert(estimate > 0);
			
				int r = sock->recv((char *)(t->data->getPtr()) + t->pos, estimate);
				if (r == -1 || r == 0) {
					LOG_ERROR(("error while reading %u bytes (r = %d)", estimate, r));
					goto disconnect;
				}
					
				t->pos += r;
				assert(t->pos <= t->len);
			
				if (t->pos == t->len) {
					if (t->size_task) {
						const char * ptr = (char *)t->data->getPtr();
						unsigned long len = ntohl(*((uint32_t *)ptr));
						unsigned long ts = ntohl(*((uint32_t *)(ptr + 4)));
						if (len > 1048576)
							throw_ex(("recv'ed packet length of %u. it seems to be far too long for regular packet (probably broken/obsoleted client)", (unsigned int)len));
						unsigned char flags = *((unsigned char *)(t->data->getPtr()) + 8);
						//LOG_DEBUG(("added task for %u bytes. flags = %02x", len, flags));
						eraseTask(_recv_q, ti);
						
						Task *t = new Task(cid, len);
						t->flags = flags;
						t->timestamp = ts;
						_recv_q.push_back(t);
					} else {
						if (t->flags & 1) {
							mrt::Chunk data;
							mrt::ZStream::decompress(data, *t->data, false);
							//LOG_DEBUG(("recv(%d, %d) (decompressed: %d)", t->id, t->data->getSize(), data.getSize()));
							*t->data = data;
						}
						_recv_q.erase(ti);

						//sdlx::Timer::microsleep("debug delay", 100000);

						sdlx::AutoMutex m2(_result_mutex);
						_result_q.push_back(t);
					}
				}
			}

			if (set.check(sock, mrt::SocketSet::Write)) {
				sdlx::AutoMutex m(_send_q_mutex);
				TaskQueue::iterator ti = findTask(_send_q, cid);
				if (ti != _send_q.end()) {
					Task *t = *ti;

					int estimate = t->len - t->pos;
					assert(estimate > 0);
					m.unlock();
					int r = sock->send((char *)(t->data->getPtr()) + t->pos, estimate);
					if (r == -1 || r == 0) {
						LOG_ERROR(("error while reading %u bytes (r = %d)", estimate, r));
						goto disconnect;
					}
					m.lock();

					t->pos += r;
					assert(t->pos <= t->len);
					if (t->pos == t->len) {
						//LOG_DEBUG(("sent %u bytes", t->len));
						eraseTask(_send_q, ti);
					}
				} else LOG_WARN(("socket was in write-set, but no-event in queue!"));
			}
		}
	}
	return 0;
} CATCH("net::Monitor::run", {})
	return 1;
}


Monitor::~Monitor() {
	_running = false;
	wait();
	LOG_DEBUG(("stopped network monitor thread."));

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

void Monitor::setCompressionLevel(const int level) {
	_comp_level = level;
}

Connection *Monitor::pop() {
	sdlx::AutoMutex m(_connections_mutex);
	ConnectionMap::iterator i = _connections.begin();
	if (i == _connections.end())
		return NULL;
	Connection *r = i->second;
	_connections.erase(i);
	return r;
}
