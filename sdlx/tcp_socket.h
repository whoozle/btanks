#ifndef __BTANKS_TCPSOCKET_H__
#define __BTANKS_TCPSOCKET_H__

#include <SDL/SDL_net.h>
#include <string>

namespace sdlx {

class TCPSocket {
public:
	TCPSocket();
	void listen(const unsigned port);
	void connect(const std::string &host, const int port);
	void close();
	
	void accept(sdlx::TCPSocket &client);
	
	const bool ready() const;

	~TCPSocket();
protected: 
	::TCPsocket _sock;
	friend class SocketSet;
};

}

#endif

