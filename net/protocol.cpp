#include "protocol.h"
#include "mrt/exception.h"
#include "sdlx/tcp_socket.h"
#include <string.h>

#include <arpa/inet.h>

Message::Message() : type(None), data_size(0) {}

Message::Message(const MessageType type) : type(type), data_size(0) {}

#define BUF_SIZE 1500

// 0, 1: short, size
// 2: type
// 3: data

void Message::send(const sdlx::TCPSocket &sock) {
	unsigned char buf[BUF_SIZE];
	if (2 + data_size >= BUF_SIZE) 
		throw_ex(("output buffer overflow: %d", 2 + data_size));
		
	buf[2] = (unsigned char)type;
	int i = writeMap((char *)(buf + 3), BUF_SIZE - 3);

	if (i + data_size >= BUF_SIZE) 
		throw_ex(("output buffer overflow: user data: %d, map data: %d", data_size, i));
	memcpy(buf + i, data, data_size);

	*(unsigned short *)buf = htons(data_size + i);

	sock.send((const void *)buf, data_size + i + 3);
	LOG_DEBUG(("message type %d, sent 3+%d+%d bytes", type, i, data_size));
}

void Message::recv(const sdlx::TCPSocket &sock) {
	unsigned char buf[2];
	_attrs.clear();
	int r = sock.recv(buf, 3);
	if (r != 3) 
		throw_ex(("fixme: implement handling of fragmented packets. read: %d", r));

	data_size = ntohs(*(unsigned short *)buf);
	type = (MessageType)buf[2];
	if (data_size > sizeof(data))
		throw_ex(("data_size exceedes data storage size. (%u)", data_size));

	if (data_size != 0) {
		sock.recv(data, data_size);
	}
	int i = readMap(data, sizeof(data));
	data_size -= i;
	if (data_size > sizeof(data) || data_size < 0)
		throw_ex(("data buffer overflow. data_size: %d", data_size));
	memmove(data, data + i, data_size);
	
	LOG_DEBUG(("message type: %d, recv %d+%d+%d bytes", type, r, i, data_size));
}

const int Message::writeMap(char *buf, const int len) const {
	int i = 0;
	
	{
		int n = _attrs.size();
		if (n > 255) 
			throw_ex(("message cannot contains more than 255 tuples"));

		*(unsigned char *)(buf + i++) = (char)n;
	}
	
	for(AttrMap::const_iterator ai = _attrs.begin(); ai != _attrs.end(); ++ai) {
		int kn = ai->first.size();
		int vn = ai->second.size();
		if (kn + vn + 2 >= len)
			throw_ex(("output buffer overrun (%d >= %d)", kn + vn + 2, len));
		
		if (kn > 255 || kn > 255)
			throw_ex(("key/value length cannot exceed 255 bytes."));
		
		*(unsigned char *)(buf + i++) = (char)kn;
		memcpy(buf + i, ai->first.c_str(), kn);
		i += kn;
		*(unsigned char *)(buf + i++) = (char)vn;
		memcpy(buf + i, ai->second.c_str(), vn);
		i += vn;
	}
	return i;
}


const int Message::readMap(const char *buf, const int len) {
	_attrs.clear();
	
	int i = 0;
	if (i >= len)
		throw_ex(("input buffer overrun (%d >= %d)", i, len));
	int n = *(unsigned char *)(buf + i++);

	LOG_DEBUG(("readMap: %d tuples", n));
	while(n--) {

		if (i >= len)
			throw_ex(("input buffer overrun (%d >= %d)", i, len));

		int kn = *(unsigned char *)(buf + i++);
		int ki = i;
		i += kn;

		if (i >= len)
			throw_ex(("input buffer overrun (%d >= %d)", i, len));
		int vn = *(unsigned char *)(buf + i++);
		int vi = i;
		i += vn;

		if (i > len)
			throw_ex(("input buffer overrun (%d >= %d)", i, len));
		LOG_DEBUG(("key: %d(%d), value: %d(%d)", ki, kn, vi, vn));
		std::string key(buf + ki, kn);
		std::string value(buf + vi, vn);	
		LOG_DEBUG(("%s = %s", key.c_str(), value.c_str()));
		_attrs[key] = value;
	}
	return i;
}


void Message::set(const std::string &key, const std::string &value) {
	_attrs[key] = value;
}

const std::string &Message::get(const std::string &key) const {
	AttrMap::const_iterator i = _attrs.find(key);
	if (i == _attrs.end())
		throw_ex(("no attribute '%s' found", key.c_str()));
	return i->second;
}
