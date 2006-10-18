#ifndef __BTANKS_PLAYER_MANAGER_H__
#define __BTANKS_PLAYER_MANAGER_H__

#include "mrt/singleton.h"
#include <vector>
#include <string>
#include "math/v3.h"
#include "alarm.h"

namespace mrt {
class Chunk;
}

namespace sdlx {
class Rect;
class Surface;
}

class PlayerSlot;
class Server;
class Client;
class Message;

class IPlayerManager {
public:
	IPlayerManager();
	DECLARE_SINGLETON(IPlayerManager);
	~IPlayerManager();
		
	void startServer();
	void startClient(const std::string &address);
	void clear();
	
	const bool isClient() const { return _client != NULL; }
	const bool isServer() const { return _server != NULL; }	

	void createControlMethod(PlayerSlot &slot, const std::string &name);

	void addSlot(const v3<int> &position);

	PlayerSlot &getSlot(const unsigned int idx);
	const PlayerSlot &getSlot(const unsigned int idx) const;
	const size_t getSlotsCount() const;
	
	void screen2world(v3<float> &pos, const int p, const int x, const int y);

	const int spawnPlayer(const std::string &classname, const std::string &animation, const std::string &method);
	void spawnPlayer(PlayerSlot &slot, const std::string &classname, const std::string &animation);

	void updatePlayers();
	void ping();
	const float extractPing(const mrt::Chunk &data) const;
	
	void setViewport(const int idx, const sdlx::Rect &rect);
	
	void tick(const float now, const float dt);
	void render(sdlx::Surface &window, const int x, const int y);
	
	const int onConnect(Message &message);
	void onMessage(const int id, const Message &message);
	void onDisconnect(const int id);	
	
private: 
	IPlayerManager(const IPlayerManager &);
	const IPlayerManager& operator=(const IPlayerManager &);

	Server *_server;
	Client *_client;

	std::vector<PlayerSlot> _players;

	float _trip_time;
	unsigned _next_ping;
	bool _ping;
	Alarm _next_sync;
};

SINGLETON(PlayerManager, IPlayerManager);

#endif
	
