#ifndef __BTANKS_CONNECTION_H__
#define __BTANKS_CONNECTION_H__

namespace sdlx  {
	class TCPSocket;
}

struct Connection {
	Connection(sdlx::TCPSocket *s);
	~Connection();
		
	sdlx::TCPSocket * sock;
};

#endif

