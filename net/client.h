#ifndef __BTANKS_CLIENT_H__
#define __BTANKS_CLIENT_H__

#include <string>

class PlayerState;
class Monitor;
class Message;

class Client {
public:
	Client();
	~Client();
	void init(const std::string &host, const unsigned port);
	void notify(const PlayerState &state);
	void send(const Message &m);
	void tick(const float dt);
	void disconnect();

protected:
	Monitor *_monitor;
};


#endif
