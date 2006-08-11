#ifndef __BTANKS_CONNECTION_H__
#define __BTANKS_CONNECTION_H__

namespace sdlx  {
	class TCPSocket;
}

class Connection {
public:
	Connection(sdlx::TCPSocket *s);
	~Connection();
		
	sdlx::TCPSocket * sock;
};

#endif

