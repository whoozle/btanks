#ifndef __BTANKS_CONNECTION_H__
#define __BTANKS_CONNECTION_H__

namespace mrt  {
	class TCPSocket;
}

class Connection {
public:
	int id;
	
	Connection(mrt::TCPSocket *s, const int id = -1);
	~Connection();
		
	mrt::TCPSocket * sock;
};

#endif

