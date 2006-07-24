#ifndef __BTANKS_TCPSOCKET_H__
#define __BTANKS_TCPSOCKET_H__

#include <SDL/SDL_net.h>
#include <string>

namespace sdlx {

class TCPSocket {
public:
	TCPSocket();
	void listen(const int port);
	void connect(const std::string &host, const int port);
	void close();

	~TCPSocket();
private: 
	::TCPsocket _sock;
};

}

#endif

