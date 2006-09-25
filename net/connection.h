#ifndef __BTANKS_CONNECTION_H__
#define __BTANKS_CONNECTION_H__

namespace mrt  {
	class TCPSocket;
}

class Connection {
public:
	Connection(mrt::TCPSocket *s);
	~Connection();
		
	mrt::TCPSocket * sock;
};

#endif

