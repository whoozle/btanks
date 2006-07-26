#ifndef __BTANKS_PROTOCOL_H__
#define __BTANKS_PROTOCOL_H__

#include <sys/types.h>

namespace sdlx {
	class TCPSocket;
}

enum MessageType {
	None,
	ServerStatus,
	PlayerEvent,
	UpdateWorld
};
	
class Message {
public: 
	Message();
	Message(const MessageType type);
	void send(const sdlx::TCPSocket &sock);
	void recv(const sdlx::TCPSocket &sock);
	
	MessageType type;
	char data[128];
	size_t data_size;
};

#endif

