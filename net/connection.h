#ifndef __BTANKS_CONNECTION_H__
#define __BTANKS_CONNECTION_H__

namespace sdlx  {
	class TCPSocket;
}

class Connection {
public:
	int id;
	
	Connection(sdlx::TCPSocket *s, const int id = -1);
	~Connection();
		
	sdlx::TCPSocket * sock;
};

#endif

