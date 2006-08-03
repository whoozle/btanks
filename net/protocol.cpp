#include "protocol.h"
#include "mrt/exception.h"
#include "mrt/chunk.h"
#include "mrt/gzip.h"
#include "mrt/serializator.h"

#include "sdlx/tcp_socket.h"
#include <string.h>

#include <arpa/inet.h>

Message::Message() : type(None) {}

Message::Message(const MessageType type) : type(type) {}

void Message::serialize(mrt::Serializator &s) const {
	s.add((int)type);
	writeMap(s);
	s.add(data);
}

void Message::deserialize(const mrt::Serializator &s) {
	int t;
	s.get(t);
	type = (MessageType) t;
	readMap(s);
	s.get(data);
}


void Message::send(const sdlx::TCPSocket &sock) const {
	mrt::Serializator s; 
	serialize(s);
	mrt::Chunk rawdata;
	{
		mrt::Chunk cdata;
		const mrt::Chunk &data = s.getData();
		mrt::ZStream::compress(cdata, data, 9);
	
		int size = cdata.getSize();
		rawdata.setSize(cdata.getSize() + 2);
	
		*(unsigned short *)(rawdata.getPtr()) = htons(size);
		memcpy((unsigned char *)rawdata.getPtr() + 2, cdata.getPtr(), cdata.getSize());
	}
	sock.send(rawdata.getPtr(), rawdata.getSize());	
	LOG_DEBUG(("message type %d, sent %d bytes (uncompressed: %d)", type, rawdata.getSize(), data.getSize()));
	//LOG_DEBUG(("message: %s", rawdata.dump().c_str()));
}

void Message::recv(const sdlx::TCPSocket &sock) {
	unsigned char buf[2];
	_attrs.clear();
	int r = sock.recv(buf, 2);
	if (r != 2) 
		throw_ex(("fixme: implement handling of fragmented packets. read: %d", r));

	int size = ntohs(*(unsigned short *)buf);
	mrt::Chunk cdata;
	cdata.setSize(size);
	sock.recv(cdata.getPtr(), size);
	
	mrt::Chunk data;
	mrt::ZStream::decompress(data, cdata);

	mrt::Serializator s(&data);
	deserialize(s);
}

void Message::writeMap(mrt::Serializator &s) const {
	int size = _attrs.size();
	s.add(size);
	for(AttrMap::const_iterator ai = _attrs.begin(); ai != _attrs.end(); ++ai) {
		s.add(ai->first);
		s.add(ai->second);
	}
}


void Message::readMap(const mrt::Serializator &s) {
	_attrs.clear();
	
	int n;
	s.get(n);

	std::string key;
	std::string value;
	
	while(n--) {
		s.get(key);
		s.get(value);
		_attrs[key] = value;
	}
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
