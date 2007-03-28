#ifndef __BTANKS_PLAYER_MANAGER_H__
#define __BTANKS_PLAYER_MANAGER_H__

/* Battle Tanks Game
 * Copyright (C) 2006-2007 Battle Tanks team
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

#include "mrt/singleton.h"
#include <vector>
#include <string>
#include <set>
#include "math/v2.h"
#include "math/v3.h"
#include "alarm.h"
#include "zbox.h"

namespace mrt {
class Chunk;
}

namespace sdlx {
class Rect;
class Surface;
}

class PlayerSlot;
class Checkpoint;
class Server;
class Client;
class Message;
class Object;

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
	const bool isServerActive() const;

	void createControlMethod(PlayerSlot &slot, const std::string &name);

	void addSlot(const v3<int> &position);
	void addCheckpoint(const v3<int> &position, const v2<int> &size, const std::string &name);
	void addHint(const v3<int> &position, const v2<int> &size, const std::string &area, const std::string &name);

	PlayerSlot &getSlot(const unsigned int idx);
	const PlayerSlot &getSlot(const unsigned int idx) const;
	PlayerSlot &getMySlot();
	const PlayerSlot &getMySlot() const;

	PlayerSlot *getSlotByID(const int id);
	const PlayerSlot *getSlotByID(const int id) const;

	const size_t getSlotsCount() const;
	
	void screen2world(v2<float> &pos, const int p, const int x, const int y);

	void getDefaultVehicle(std::string &vehicle, std::string &animation);
	
	const int findEmptySlot() const;
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
	
	void onPlayerDeath(const Object *player, const Object *killer);
	void gameOver(const std::string &reason, const float time);
	
	void onDestroyMap(const std::set<v3<int> > & cells);
	
	void validateViewports();

private: 
	void serializeSlots(mrt::Serializator &s) const;
	void deserializeSlots(const mrt::Serializator &s);
	
	void broadcast(const Message &m);
	

	IPlayerManager(const IPlayerManager &);
	const IPlayerManager& operator=(const IPlayerManager &);

	Server *_server;
	Client *_client;

	int _my_idx;
	std::vector<PlayerSlot> _players;
	std::vector<Checkpoint> _checkpoints;
	typedef std::vector<std::pair<ZBox, std::pair<std::string, std::string> > > Hints; 
	Hints _hints;

	float _trip_time;
	unsigned _next_ping;
	bool _ping;
	Alarm _next_sync;
};

SINGLETON(PlayerManager, IPlayerManager);

#endif
	
