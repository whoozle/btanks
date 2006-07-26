#ifndef __BTANKS_PROTOCOL_H__
#define __BTANKS_PROTOCOL_H__

#include <sys/types.h>
#include <map>
#include <string>

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
	
	void set(const std::string &key, const std::string &value);
	const std::string &get(const std::string &key) const;
	
	MessageType type;
	char data[128];
	size_t data_size;
	
private:
	typedef std::map<const std::string, std::string> AttrMap;
	AttrMap _attrs;
};

#endif

