#ifndef __BTANKS_CONNECTION_H__
#define __BTANKS_CONNECTION_H__

namespace sdlx  {
	class TCPSocket;
}

struct Connection {
	Connection(sdlx::TCPSocket *s) : sock(s) {}
	~Connection() { delete sock; sock = NULL; }
		
	sdlx::TCPSocket * sock;
};

#endif

