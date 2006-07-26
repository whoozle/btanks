#include "protocol.h"
#include "mrt/exception.h"
#include "sdlx/tcp_socket.h"
#include <string.h>

#include <arpa/inet.h>

Message::Message() : type(None), data_size(0) {}

Message::Message(const MessageType type) : type(type), data_size(0) {}

#define BUF_SIZE 256

// 0, 1: short, size
// 2: type
// 3: data

void Message::send(const sdlx::TCPSocket &sock) {
	unsigned char buf[BUF_SIZE];
	if (2 + data_size >= BUF_SIZE) 
		throw_ex(("output buffer overflow"));
		
	*(unsigned short *)buf = htons(data_size);
	buf[2] = (unsigned char)type;
	memcpy(buf + 3, data, data_size);
	sock.send((const void *)buf, data_size + 3);
	LOG_DEBUG(("message type %d, sent %d bytes", type, data_size + 3));
}

void Message::recv(const sdlx::TCPSocket &sock) {
	unsigned char buf[2];
	if (sock.recv(buf, 3) != 3) 
		throw_ex(("fixme: implement handling of fragmented packets."));
	data_size = ntohs(*(unsigned short *)buf);
	type = (MessageType)buf[2];
	if (data_size != 0) {
		sock.recv(data, data_size);
	}
	LOG_DEBUG(("message type: %d, recv %d bytes", type, data_size));
}
