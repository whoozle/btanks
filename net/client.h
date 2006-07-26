#ifndef __BTANKS_CLIENT_H__
#define __BTANKS_CLIENT_H__

#include "sdlx/tcp_socket.h"
#include <string>

class PlayerState;
class Client {
public:
	Client();
	void init(const std::string &host, const unsigned port);
	void notify(const PlayerState &state);
	void tick(const float dt);

protected:
	sdlx::TCPSocket _sock;
	bool _running;
};


#endif
