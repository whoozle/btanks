#ifndef __BTANKS_CLIENT_H__
#define __BTANKS_CLIENT_H__

#include <string>

class PlayerState;
class Connection;

class Client {
public:
	Client();
	void init(const std::string &host, const unsigned port);
	void notify(const PlayerState &state);
	void tick(const float dt);

protected:
	Connection * _conn;
	bool _running;
};


#endif
