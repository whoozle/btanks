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

#include "export_btanks.h"
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
class SpecialZone;
class Server;
class Client;
class Message;
class Object;

class BTANKSAPI IPlayerManager {
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

	void addSlot(const v3<int> &position);
	void addSpecialZone(const SpecialZone& zone);

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

	void updatePlayers();
	void ping();
	const float extractPing(const mrt::Chunk &data) const;
	
	void setViewport(const int idx, const sdlx::Rect &rect);
	
	void tick(const unsigned int now, const float dt);
	void render(sdlx::Surface &window, const int x, const int y);
	
	const int onConnect(Message &message);
	void onMessage(const int id, const Message &message);
	void onDisconnect(const int id);	
	
	void onPlayerDeath(const Object *player, const Object *killer);
	void gameOver(const std::string &reason, const float time);
	
	void onDestroyMap(const std::set<v3<int> > & cells);
	
	void validateViewports();
	
	//for special zones
	void send(const int id, const Message & msg);
	
	void updateControls(); //recreates control methods.

private: 
	void serializeSlots(mrt::Serializator &s) const;
	void deserializeSlots(const mrt::Serializator &s);
	
	void broadcast(const Message &m);
	

	IPlayerManager(const IPlayerManager &);
	const IPlayerManager& operator=(const IPlayerManager &);

	Server *_server;
	Client *_client;

	int _my_idx;

	std::set<int> _global_zones_reached;
	std::vector<PlayerSlot> _players;
	std::vector<SpecialZone> _zones;

	float _trip_time;
	unsigned _next_ping;
	bool _ping;
	Alarm _next_sync;
	bool _game_joined;
};

SINGLETON(BTANKSAPI, PlayerManager, IPlayerManager);

#endif
	
