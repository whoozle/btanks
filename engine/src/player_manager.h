#ifndef __BTANKS_PLAYER_MANAGER_H__
#define __BTANKS_PLAYER_MANAGER_H__

/* Battle Tanks Game
 * Copyright (C) 2006-2009 Battle Tanks team
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

/* 
 * Additional rights can be granted beyond the GNU General Public License 
 * on the terms provided in the Exception. If you modify this file, 
 * you may extend this exception to your version of the file, 
 * but you are not obligated to do so. If you do not wish to provide this
 * exception without modification, you must delete this exception statement
 * from your version and license this file solely under the GPL without exception. 
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
#include "netstats.h"

//could be forward, but incompatible with stlport
#include "special_zone.h"
#include "player_slot.h"
#include "sl08/sl08.h"
#include "mrt/sys_socket.h"

namespace mrt {
class Chunk;
}

namespace sdlx {
class Rect;
class Surface;
}

class PlayerState;
class Server;
class Client;
class Message;
class Object;

class BTANKSAPI IPlayerManager {
public:
	IPlayerManager();
	DECLARE_SINGLETON(IPlayerManager);
	~IPlayerManager();
		
	void start_server();
	void start_client(const mrt::Socket::addr &address, const size_t n);
	void clear(bool disconnect = false);
	
	inline const bool is_client() const { return _client != NULL; }
	inline const bool is_server() const { return _server != NULL; }	
	const bool is_server_active() const;
	void disconnect_all();

	void add_slot(const v3<int> &position);
	void add_special_zone(const SpecialZone& zone);

	PlayerSlot *get_my_slot(); //remove me

	PlayerSlot &get_slot(const unsigned int idx);
	const PlayerSlot &get_slot(const unsigned int idx) const;

	const int get_slot_id(const int object_id) const;
	PlayerSlot *get_slot_by_id(const int id);
	const PlayerSlot *get_slot_by_id(const int id) const;

	const size_t get_slots_count() const;
	const size_t get_free_slots_count() const;
	
	const int find_empty_slot();
	const int spawn_player(const std::string &classname, const std::string &animation, const std::string &method);

	void update_players(const float dt);
	void ping();
	
	void tick(const float dt);
	void render(sdlx::Surface &window, const int x, const int y);
	
	const int on_connect();
	void on_message(const int id, const Message &message);
	void on_disconnect(const int id);	
	void onMap();
	
	void game_over(const std::string &area, const std::string &message, const float time);
	
	void on_destroy_map(const std::set<v3<int> > & cells);
	
	void validate_viewports();
	
	//for special zones
	void send(const PlayerSlot &slot, const Message & msg);
	void say(const std::string &message);
	void action(const PlayerSlot &slot, const std::string &type, const std::string &subtype = std::string(), const PlayerSlot * killer_slot = NULL);
	
	void broadcast_message(const std::string &area, const std::string &message, const float duration);
	void send_hint(const int slot_id, const std::string &area, const std::string &message);
	
	void update_controls(); //recreates control methods.
	
	const SpecialZone& get_next_checkpoint(PlayerSlot &slot); 
	void fix_checkpoints(PlayerSlot &slot, const SpecialZone &zone); 
	
	void send_object_state(const int id, const PlayerState & state);
	void request_objects(const int first_id);

private: 
	void onPlayerDeath(const Object *player, const Object *killer);
	
	sl08::slot1<void, const std::set<v3<int> > &, IPlayerManager> on_destroy_map_slot;
	sl08::slot0<void, IPlayerManager> on_load_map_slot;
	sl08::slot2<void, const Object *, const Object *, IPlayerManager> on_object_death_slot;
	
	void serialize_slots(mrt::Serializator &s) const;
	void deserialize_slots(const mrt::Serializator &s);
	
	void broadcast(const Message &m, const bool per_connection);
	

	IPlayerManager(const IPlayerManager &);
	const IPlayerManager& operator=(const IPlayerManager &);

	Server *_server;
	Client *_client;

	size_t _local_clients;

	std::set<int> _global_zones_reached;
	std::vector<PlayerSlot> _players;
	std::vector<SpecialZone> _zones;

	NetStats _net_stats;
	
	unsigned _next_ping;
	bool _ping;
	Alarm _next_sync;
	bool _game_joined;
	mrt::Socket::addr _recent_address;
	
	typedef std::set<int> ObjectStates;
	ObjectStates _object_states;
	
	int _connection_id;
};

PUBLIC_SINGLETON(BTANKSAPI, PlayerManager, IPlayerManager);

#endif
	
