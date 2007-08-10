
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
#include <string>

#include "player_manager.h"
#include "player_slot.h"
#include "object.h"
#include "world.h"
#include "config.h"
#include "resource_manager.h"
#include "version.h"
#include "game.h"
#include "window.h"
#include "game_monitor.h"
#include "special_zone.h"
#include "nickname.h"
#include "menu/tooltip.h"
#include "menu/chat.h"

#include "controls/control_method.h"

#include "net/server.h"
#include "net/client.h"
#include "net/protocol.h"

#include "sound/mixer.h"
#include "sdlx/surface.h"
#include "tmx/map.h"

#include "mrt/random.h"

#include "math/unary.h"
#include "math/binary.h"

IMPLEMENT_SINGLETON(PlayerManager, IPlayerManager);


const int IPlayerManager::onConnect(Message &message) {
	/*
	const std::string an = "red-tank";
	LOG_DEBUG(("new client! spawning player:%s", an.c_str()));
	const int client_id = spawnPlayer("tank", an, "network");
	LOG_DEBUG(("client #%d", client_id));
	*/
	const int client_id = findEmptySlot();
	
	LOG_DEBUG(("sending server status message..."));
	message.type = Message::ServerStatus;
	message.set("version", getVersion());

	mrt::Serializator s;
	Map->serialize(s);
	
	s.add(client_id);
	//_players[client_id].position.serialize(s);
	
	serializeSlots(s);	

	message.data = s.getData();
	LOG_DEBUG(("server status message size = %u", (unsigned) message.data.getSize()));
	_players[client_id].reserved = true;
	//LOG_DEBUG(("world: %s", message.data.dump().c_str()));
	return client_id;
}

void IPlayerManager::onDisconnect(const int id) {
	if ((unsigned)id >= _players.size()) {
		LOG_ERROR(("player %d doesnt exists, so cannot disconnect.", id));
		return;
	}
	PlayerSlot &slot = _players[id];
	Object *obj = slot.getObject();
	if (obj)
		obj->Object::emit("death", NULL);
	
	slot.clear();
}


void IPlayerManager::onMessage(const int id, const Message &message) {
TRY {
	LOG_DEBUG(("incoming message %s", message.getType()));
	switch(message.type) {
	case Message::ServerStatus: {
		LOG_DEBUG(("server version: %s", message.get("version").c_str()));
		LOG_DEBUG(("loading map..."));
		
		mrt::Serializator s(&message.data);
		Map->deserialize(s);
		GameMonitor->loadMap(NULL, Map->getName(), false, true);
		
		s.get(_my_idx);
		LOG_DEBUG(("my_idx = %d", _my_idx));
		
		int n;
		s.get(n);
		if (_my_idx >= n) 
			throw_ex(("server returned bogus player index (%d/%d)", _my_idx, n));
		
		_players.clear();
		for(int i = 0; i < n; ++i) {
			PlayerSlot slot;
			slot.deserialize(s);
			_players.push_back(slot);
		}
		
		PlayerSlot &slot = _players[_my_idx];
		
		slot.viewport.reset();
		slot.visible = true;
		
		assert(slot.control_method == NULL);
		GET_CONFIG_VALUE("player.control-method", std::string, control_method, "keys");	
		slot.createControlMethod(control_method);

		LOG_DEBUG(("players = %u", (unsigned)_players.size()));
		
		Message m(Message::RequestPlayer);

		std::string vehicle;
		Config->get("menu.default-vehicle-1", vehicle, "launcher");

		m.set("vehicle", vehicle);

		std::string name;
		Config->get("player.name-1", name, Nickname::generate());
		m.set("name", name);
		
		_client->send(m);
		
		_next_ping = 0;
		_ping = true;
		break;	
	}
	
	case Message::RequestPlayer: {
		int n = (int)_players.size();
		if (id >= n) 
			throw_ex(("connection id %d exceedes player count %u", id, n));
		PlayerSlot &slot = _players[id];
		if (!slot.reserved) 
			throw_ex(("RequestPlayer sent over non-reserved slot[%d]. bug/hack.", id));	
		
		std::string vehicle, animation;
		Config->get("multiplayer.restrict-start-vehicle", vehicle, "");
		Config->get("multiplayer.restrict-start-animation", animation, "");

		if (vehicle.empty()) {	
			vehicle = message.get("vehicle");
		}

		if (animation.empty()) {
			if (vehicle == "tank" || vehicle == "launcher" || vehicle == "shilka") {
				static const char * colors[4] = {"green", "red", "yellow", "cyan"};
				animation = colors[mrt::random(4)];
				animation += "-" + vehicle;
			} else animation = vehicle;
		} 

		slot.name = message.get("name");
		LOG_DEBUG(("player%d: %s:%s, name: %s", id, vehicle.c_str(), animation.c_str(), slot.name.c_str()));

		slot.remote = true;
		slot.reserved = false;
		
		slot.spawnPlayer(vehicle, animation);

		mrt::Serializator s;
		World->serialize(s);
		serializeSlots(s);
		
		Message m(Message::GameJoined);
		m.data = s.getData();
		_server->send(id, m);

		Window->resetTimer();
		break;
	}
	
	case Message::GameJoined: {
		assert(_my_idx >= 0);
		mrt::Serializator s(&message.data);
		World->deserialize(s);
		deserializeSlots(s);

		Window->resetTimer();
		_game_joined = true;
		break;
	}
	
	case Message::UpdateWorld: {
		mrt::Serializator s(&message.data);
		deserializeSlots(s);
		World->applyUpdate(s, _trip_time / 1000.0);
		GameMonitor->deserialize(s);
		break;
	} 
	case Message::PlayerState: {
		mrt::Serializator s(&message.data);
		if (id < 0 || (unsigned)id >= _players.size())
			throw_ex(("player id exceeds players count (%d/%d)", id, (int)_players.size()));
		PlayerSlot &slot = _players[id];
		if (slot.reserved) 
			throw_ex(("player sent PlayerState message before join. bye."));
/*		ExternalControl * ex = dynamic_cast<ExternalControl *>(slot.control_method);
		if (ex == NULL)
			throw_ex(("player with id %d uses non-external control method", id));
		
		ex->state.deserialize(s);
		*/
		PlayerState state;
		state.deserialize(s);

		Object * obj = slot.getObject();
		if (obj == NULL) {
			LOG_WARN(("player state for non-existent object %d recv'ed", slot.id));
			break;
		}
	
		assert(slot.id == obj->getID());

		World->tick(*obj, -slot.trip_time / 1000.0, false);
		
		slot.need_sync = obj->updatePlayerState(state);
		if (slot.need_sync == false) {
			LOG_WARN(("player %d send duplicate player state. %s", id, state.dump().c_str()));
			slot.need_sync = true;
		}
		
		World->tick(*obj, slot.trip_time / 1000.0, false);
		break;
	} 
	case Message::UpdatePlayers: { 
		mrt::Serializator s(&message.data);
		IWorld::ObjectMap updated_objects, interpolated_objects;
		while(!s.end()) {
			int id;
			s.get(id);
			unsigned slot;
			for(slot = 0; slot < _players.size(); ++slot) {
				if (_players[slot].id == id)
					break;
			}
			if (slot >= _players.size()) 
				LOG_WARN(("object id %u was not found in slots", slot));
			const bool my_state = slot < _players.size() && slot == (unsigned)_my_idx;
			
			Object *o = World->getObjectByID(id);

			PlayerState state; 
			state.deserialize(s);
			
			if (o != NULL) { 
				World->tick(*o, -_trip_time / 1000.0, false);
			}
			
			World->deserializeObjectPV(s, o);
			bool dont_interpolate;
			s.get(dont_interpolate);
			
			if (o == NULL) {
				LOG_WARN(("nothing known about object id %d now, skip update", id));
				continue;
			}

			LOG_DEBUG(("slot: %d, id: %d, state: %s %s (my state: %s) %s", 
				slot, id, state.dump().c_str(), my_state?"[skipped]":"", o->getPlayerState().dump().c_str(), 
				(my_state && state != o->getPlayerState())?"**DIFFERS**":""));

			if (!my_state)
				o->updatePlayerState(state); //update states for all players but me.

			updated_objects.insert(IWorld::ObjectMap::value_type(o->getID(), o));
			if (!dont_interpolate)
				interpolated_objects.insert(IWorld::ObjectMap::value_type(o->getID(), o));
			else o->uninterpolate();
		}	
		World->tick(updated_objects, _trip_time / 1000.0, false);
		World->interpolateObjects(interpolated_objects);
		break;
	} 
	case Message::Ping: {
		Message m(Message::Pang);
		m.data = message.data;
		size_t size = m.data.getSize();
		m.data.reserve(size + sizeof(unsigned int));
		
		unsigned int ts = SDL_GetTicks();
		*(unsigned int *)((unsigned char *)m.data.getPtr() + size) = ts;
		_server->send(id, m);
		break;
	}
	
	case Message::Pang: {
		const mrt::Chunk &data = message.data;
		float ping = extractPing(data);
		GET_CONFIG_VALUE("multiplayer.ping-interpolation-multiplier", int, pw, 3);
		_trip_time = (pw * ping + _trip_time) / (pw + 1);
		
		GET_CONFIG_VALUE("multiplayer.ping-interval", int, ping_interval, 1500);

		_next_ping = SDL_GetTicks() + ping_interval; 
		
		LOG_DEBUG(("ping = %g", _trip_time));
		Message m(Message::Pong);
		m.data.setData((unsigned char *)data.getPtr() + sizeof(unsigned int), data.getSize() - sizeof(unsigned int));
		_client->send(m);
		break;
	}
	
	case Message::Pong: {
		if (id < 0 || (unsigned)id >= _players.size())
			throw_ex(("player id exceeds players count (%d/%d)", id, (int)_players.size()));
		float ping = extractPing(message.data);
		
		GET_CONFIG_VALUE("multiplayer.ping-interpolation-multiplier", int, pw, 3);
		_players[id].trip_time = (pw * ping + _players[id].trip_time) / (pw + 1);
		LOG_DEBUG(("player %d: ping: %g ms", id, ping));		
		break;
	}
	
	case Message::Respawn: {
		TRY {
		if (id < 0 || (unsigned)id >= _players.size())
			throw_ex(("player id exceeds players count (%d/%d)", id, (int)_players.size()));
		mrt::Serializator s(&message.data);
		deserializeSlots(s);
		World->applyUpdate(s, _trip_time / 1000.0);
		} CATCH("on-message(respawn)", throw;);
	break;
	}
	case Message::GameOver: {
		TRY {
			GameMonitor->gameOver("messages", message.get("message"), atof(message.get("duration").c_str()) - _trip_time / 1000.0, false);
		} CATCH("on-message(gameover)", throw; )
		break;
	}
	
	case Message::TextMessage: {
		TRY {
			GameMonitor->displayMessage("messages", message.get("message"), atof(message.get("duration").c_str()) - _trip_time / 1000.0);
		} CATCH("on-message(text-message)", throw; )		
		break;
	}
	case Message::DestroyMap: {
		mrt::Serializator s(&message.data);
		int n;
		s.get(n);
		LOG_DEBUG(("%d destroyed map cells", n));
		v3<int> cell;
		while(n--) {
			cell.deserialize(s);
			Map->_destroy(cell.z, v2<int>(cell.x, cell.y));
		}
		break;
	}
	case Message::PlayerMessage: {
		if (id < 0 || (unsigned)id >= _players.size())
			throw_ex(("player id exceeds players count (%d/%d)", id, (int)_players.size()));

		if (_client)
			Game->getChat()->addMessage(message.get("nick"), message.get("text"));

		if (_server) {
			std::string nick =  _players[id].name;
			Game->getChat()->addMessage(nick, message.get("text"));
			Message msg(message);
			msg.set("nick", nick);
			broadcast(msg);
		}
		
		} break;
	
	default:
		LOG_WARN(("unhandled message: %s\n%s", message.getType(), message.data.dump().c_str()));
	};
} CATCH("onMessage", { 
	if (_server) 
		_server->disconnect(id);
	if (_client) {
		_client->disconnect();
		Game->clear();
		GameMonitor->displayMessage("errors", "multiplayer-exception", 1);
	}
});
}



void IPlayerManager::ping() {
	Message m(Message::Ping);
	unsigned int ts = SDL_GetTicks();
	LOG_DEBUG(("ping timestamp = %u", ts));
	m.data.setData(&ts, sizeof(ts));
	_client->send(m);
}


void IPlayerManager::updatePlayers() {
	if (_client && !_game_joined)
		return;
	
TRY {

	int n = _players.size();

	for(int i = 0; i < n; ++i) {
		PlayerSlot &slot = _players[i];
		if (slot.id <= 0)
			continue;
		
		Object *o = slot.getObject();
		if (o != NULL /* && !o->isDead() */) {
			//server or split screen
			v3<int> player_pos;
			{
				v2<int> p;
				o->getCenterPosition(p);
				player_pos.x = p.x;
				player_pos.y = p.y;
				player_pos.z = o->getZ();
			}
			
			if (isClient())
				continue;
			
			//check for Special Zones ;)
			
			size_t cn = _zones.size();
			for(size_t c = 0; c < cn; ++c) {
				SpecialZone &zone = _zones[c];
				bool global = zone.global();
				if (zone.in(player_pos) && 
					_global_zones_reached.find(c) == _global_zones_reached.end() && 
					slot.zones_reached.find(c) == slot.zones_reached.end()) {
					
					LOG_DEBUG(("player[%d] zone %u reached.", i, (unsigned)c));
					slot.zones_reached.insert(c);
					if (global)
						_global_zones_reached.insert(c);
					
					zone.onEnter(i);
				}
			}
			
			continue;
		}
		
		if (slot.spawn_limit > 0) {
			if (--slot.spawn_limit <= 0) {
				slot.clear();
				GameMonitor->gameOver("messages", "game-over", 5, false);
				continue;
			}
			LOG_DEBUG(("%d lives left", slot.spawn_limit));
		}
		
		LOG_DEBUG(("player in slot %d is dead. respawning. frags: %d", i, slot.frags));

		slot.spawnPlayer(slot.classname, slot.animation);

		if (slot.getObject()) {
			Mixer->playSample(slot.getObject(), "respawn.ogg", false);
		}
		
		if (isServer() && slot.remote) {
			Message m(Message::Respawn);
			mrt::Serializator s;
			serializeSlots(s);
			World->generateUpdate(s, false);
			
			m.data = s.getData();
			_server->send(i, m);
		}
	}
	
	bool updated = false;
	
	for(int i = 0; i < n; ++i) {
		PlayerSlot &slot = _players[i];
		Object *obj = slot.getObject();

		if (slot.control_method != NULL && obj != NULL) {
			PlayerState state = obj->getPlayerState();
			bool hint = state.hint_control;
			slot.control_method->updateState(slot, state);
			
			if (obj->updatePlayerState(state)) {
				updated = true;
				slot.need_sync = true;
				if (state.hint_control && !hint) {
					slot.displayLast();
				}
			}
		} else if (slot.control_method == NULL && obj != NULL) {
			if (obj->getPlayerState() != slot.old_state) {
				slot.old_state = obj->getPlayerState();
				slot.need_sync = true;
			}
		}
		
		if (slot.need_sync)
			updated = true;
		
		
	}
				
	if (_client && _players.size() != 0 && _players[_my_idx].need_sync)	{
		mrt::Serializator s;
		PlayerSlot &slot = _players[_my_idx];
		
		const Object * o = slot.getObject();
		if (o != NULL) {
			o->getPlayerState().serialize(s);
		
			Message m(Message::PlayerState);
			m.data = s.getData();
			_client->send(m);
			_players[_my_idx].need_sync = false;
		}
	}
	//cross-players state exchange


	if (_server && updated) {
		bool send = false;
		mrt::Serializator s;

		for(int j = 0; j < n; ++j) {

			PlayerSlot &slot = _players[j];
			if (slot.id >= 0 && slot.need_sync) {
				//LOG_DEBUG(("object in slot %d: %s (%d) need sync", j, slot.obj->registered_name.c_str(), slot.obj->getID()));
				s.add(slot.id);
				Object * o = slot.getObject();
				assert(o != NULL);
				o->getPlayerState().serialize(s);
				World->serializeObjectPV(s, o);
				s.add(slot.dont_interpolate);
				send = true;
				slot.need_sync = false;
				slot.dont_interpolate = false;
			}
		}

		if (send) {
			Message m(Message::UpdatePlayers);
			m.data = s.getData();
			broadcast(m);
		}
	}
} CATCH("updatePlayers", {
		Game->clear();
		GameMonitor->displayMessage("errors", "multiplayer-exception", 1);
})
}


const float IPlayerManager::extractPing(const mrt::Chunk &data) const {
	if (data.getSize() < sizeof(unsigned int))
		throw_ex(("invalid pong recv'ed. (size: %u)", (unsigned)data.getSize()));
	
	unsigned int ts = * (unsigned int *)data.getPtr();
	Uint32 ticks = SDL_GetTicks();
	float delta = (int)(ticks - ts);
	if (delta < 0) delta = -delta; //wrapped around.
	if (delta > 60000)
		throw_ex(("server returns bogus timestamp value. [%g]", delta));
	delta /= 2;
	return delta;
}



IPlayerManager::IPlayerManager() : 
	_server(NULL), _client(NULL), _my_idx(-1), _players(), _trip_time(10), _next_ping(0), _ping(false), _next_sync(true), _game_joined(false)
{}

IPlayerManager::~IPlayerManager() {}

void IPlayerManager::startServer() {
	clear();
	 
	_server = new Server;
	_server->init(9876);
}

void IPlayerManager::startClient(const std::string &address) {
	clear();
	
	World->setSafeMode(true);
	unsigned port = 9876;
	TRY {
		_client = new Client;
		_client->init(address, port);
	} CATCH("_client.init", { delete _client; _client = NULL; throw; });

}
void IPlayerManager::clear() {
	LOG_DEBUG(("deleting server/client if exists."));
	_ping = false;
	_game_joined = false;
	delete _server; _server = NULL;
	delete _client; _client = NULL;
	_my_idx = -1;

	GET_CONFIG_VALUE("multiplayer.sync-interval", float, sync_interval, 103.0/101);
	_next_sync.set(sync_interval);

	LOG_DEBUG(("cleaning up players..."));
	_global_zones_reached.clear();
	_players.clear();	
	_zones.clear();
}

void IPlayerManager::addSlot(const v3<int> &position) {
	PlayerSlot slot;
	slot.position = position;
	_players.push_back(slot);
}

void IPlayerManager::addSpecialZone(const SpecialZone &zone) {
	if (zone.size.x == 0 || zone.size.y == 0)
		throw_ex(("zone size cannot be 0"));
	LOG_DEBUG(("adding zone '%s' named '%s' at %d %d (%dx%d)", zone.type.c_str(), zone.name.c_str(), zone.position.x, zone.position.y, zone.size.x, zone.size.y));
	_zones.push_back(zone);
}

PlayerSlot &IPlayerManager::getSlot(const unsigned int idx) {
	if (idx >= _players.size())
		throw_ex(("slot #%u does not exist", idx));
	return _players[idx];
}
const PlayerSlot &IPlayerManager::getSlot(const unsigned int idx) const {
	if (idx >= _players.size())
		throw_ex(("slot #%u does not exist", idx));
	return _players[idx];
}
PlayerSlot *IPlayerManager::getMySlot() {
	if (_my_idx < 0) 
		return NULL;
	if (_my_idx >= (int)_players.size())
		throw_ex(("slot #%u does not exist", _my_idx));
	return &_players[_my_idx];
}
const PlayerSlot *IPlayerManager::getMySlot() const {
	if (_my_idx < 0) 
		return NULL;
	if (_my_idx >= (int)_players.size())
		throw_ex(("slot #%u does not exist", _my_idx));
	return &_players[_my_idx];
}


PlayerSlot *IPlayerManager::getSlotByID(const int id) {
	if (id <= 0)
		return NULL;
	
	for(std::vector<PlayerSlot>::iterator i = _players.begin(); i != _players.end(); ++i) {
		PlayerSlot &slot = *i;
		if (slot.id == id) 
			return &*i;
	}
	return NULL;
}
const PlayerSlot *IPlayerManager::getSlotByID(const int id) const {
	for(std::vector<PlayerSlot>::const_iterator i = _players.begin(); i != _players.end(); ++i) {
		const PlayerSlot &slot = *i;
		if (slot.id == id) 
			return &*i;
	}
	return NULL;
}


const int IPlayerManager::findEmptySlot() const {
	int i, n = _players.size();
	for(i = 0; i < n; ++i) {
		if (_players[i].id == -1 && !_players[i].reserved)
			break;
	}
	if (i == n) 
		throw_ex(("no available slots found from %d", n));
	return i;
}

const int IPlayerManager::spawnPlayer(const std::string &classname, const std::string &animation, const std::string &control_method) {
	int i = findEmptySlot();
	PlayerSlot &slot = _players[i];

	slot.createControlMethod(control_method);
	
	LOG_DEBUG(("player: %s.%s using control method: %s", classname.c_str(), animation.c_str(), control_method.c_str()));
	slot.spawnPlayer(classname, animation);
	return i;
}


void IPlayerManager::setViewport(const int idx, const sdlx::Rect &rect) {
	PlayerSlot &slot = _players[idx];
	if (_my_idx < 0)
		_my_idx = idx;
	
	slot.visible = true;
	slot.viewport = rect;
	const Object *o = _players[idx].getObject();
	if (o == NULL)
		throw_ex(("setViewport %d called on empty slot.", idx));
	
	v2<float> pos = o->getCenterPosition();
	slot.map_pos.x = (int)pos.x - rect.w / 2;
	slot.map_pos.y = (int)pos.y - rect.h / 2;
}

void IPlayerManager::validateViewports() {
		if (Map->loaded()) {
			for(unsigned p = 0; p < _players.size(); ++p) {
				PlayerSlot &slot = _players[p];
				if (!slot.visible) 
					continue;
				
				slot.validatePosition(slot.map_pos);				
			}
		}
}

void IPlayerManager::tick(const unsigned int now, const float dt) {
TRY {
	if (_server) {
		if (_next_sync.tick(dt) && isServerActive()) {
			Message m(Message::UpdateWorld);
			{
				mrt::Serializator s;
				serializeSlots(s);
				World->generateUpdate(s, true);
				GameMonitor->serialize(s);
				m.data = s.getData();
			}
			LOG_DEBUG(("sending world update... (size: %u)", (unsigned)m.data.getSize()));
			broadcast(m);
		}
		_server->tick(dt);
	}

	if (_client) {
		_client->tick(dt);
		if (_ping && now >= _next_ping) {
			ping();
			GET_CONFIG_VALUE("multiplayer.ping-interval", int, ping_interval, 1500);
			_next_ping = (int)(now + ping_interval); //fixme: hardcoded value
		}
	}
} CATCH("tick", {
		Game->clear();
		GameMonitor->displayMessage("errors", "multiplayer-exception", 1);
		return;
})
	//bool listener_set = false;
	v2<float> listener_pos, listener_vel, listener_size;
	float listeners = 0;
	for(size_t pi = 0; pi < _players.size(); ++pi) {
		PlayerSlot &slot = _players[pi];
		if (!slot.visible)
			continue;
		const Object * o = slot.getObject();
		if (o == NULL)
			continue;

		v2<float> pos, vel;
		o->getInfo(pos, vel);
		listener_pos += pos;
		listener_vel += vel;
		listeners += 1;
		listener_size += o->size;
	}

	if (listeners > 0) {
		listener_pos /= listeners;
		listener_vel /= listeners;
		listener_size /= listeners;
		Mixer->setListener(listener_pos.convert2v3(0), listener_vel.convert2v3(0), listener_size.length());
	}

	for(size_t pi = 0; pi < _players.size(); ++pi) {
		PlayerSlot &slot = _players[pi];
		slot.tick(dt);
	}

	validateViewports();
}


void IPlayerManager::render(sdlx::Surface &window, const int vx, const int vy) {

		for(unsigned p = 0; p < _players.size(); ++p) {
			PlayerSlot &slot = _players[p];
			if (!slot.visible)
				continue;

			if (slot.viewport.w == 0) 
				slot.viewport = window.getSize();
				
			slot.viewport.x += vx;
			slot.viewport.y += vy;

			GET_CONFIG_VALUE("player.controls.immediate-camera-sliding", bool, ics, false);		
			
			v2<float> pos = ics?slot.map_pos + slot.map_dpos.convert<float>() : slot.map_pos;
			slot.validatePosition(pos);
			
			World->render(window, sdlx::Rect((int)pos.x, (int)pos.y, slot.viewport.w, slot.viewport.h), 
				slot.viewport, -10000, 10001, slot.getObject());

			GET_CONFIG_VALUE("engine.show-special-zones", bool, ssz, false);
			if (ssz) {
				for(size_t i = 0; i < _zones.size(); ++i) {
					sdlx::Rect pos(_zones[i].position.x, _zones[i].position.y, _zones[i].size.x, _zones[i].size.y);
					static sdlx::Surface zone;
					if (zone.isNull()) {
						//zone.createRGB(_zones[i].size.x, _zones[i].size.y, 32); 
						zone.createRGB(32, 32, 32); 
						zone.convertAlpha();
						zone.fill(zone.mapRGBA(255, 0, 0, 51));
					}

					pos.x -= (int)slot.map_pos.x;
					pos.y -= (int)slot.map_pos.y;
					for(int y = 0; y <= (_zones[i].size.y - 1) / zone.getHeight(); ++y) 
						for(int x = 0; x <= (_zones[i].size.x - 1) / zone.getWidth(); ++x) 
						window.copyFrom(zone, pos.x + x * zone.getWidth(), pos.y + y * zone.getHeight());
				}
			}
			
			if (!slot.tooltips.empty()) {
				Tooltip *t = slot.tooltips.front().second;
				int w, h;
				t->getSize(w, h);
				t->render(window, slot.viewport.x, slot.viewport.h - h);
			}
	
			slot.viewport.x -= vx;
			slot.viewport.y -= vy;
		}
}

void IPlayerManager::screen2world(v2<float> &pos, const int p, const int x, const int y) {
	PlayerSlot &slot = _players[p];
	pos.x = slot.map_pos.x + x;
	pos.y = slot.map_pos.x + y;
}

const size_t IPlayerManager::getSlotsCount() const {
	return _players.size();
}

void IPlayerManager::serializeSlots(mrt::Serializator &s) const {
	int n = _players.size();
	s.add(n);
	for(int i = 0; i < n; ++i) {
		_players[i].serialize(s);
	}
	n = _global_zones_reached.size();
	s.add(n);
	for(std::set<int>::const_iterator i = _global_zones_reached.begin(); i != _global_zones_reached.end(); ++i) {
		s.add(*i);
	}
}

void IPlayerManager::deserializeSlots(const mrt::Serializator &s) {
	int n, pn = _players.size();
	s.get(n);
	assert(pn == n);
	
	for(int i = 0; i < n; ++i) {
		_players[i].deserialize(s);
	}
	s.get(n);
	_global_zones_reached.clear();
	while(n--) {
		int z; 
		s.get(z);
		_global_zones_reached.insert(z);
	}
}

void IPlayerManager::broadcast(const Message &m) {
	int n = _players.size();
	for(int i = 0; i < n; ++i) {
		const PlayerSlot &slot = _players[i];
		if (slot.remote && !slot.reserved && slot.id != -1) 
			_server->send(i, m);
	}
}

void IPlayerManager::send(const int id, const Message & msg) {
	if (_server == NULL)
		throw_ex(("PlayerManager->send() allowed only in server mode"));
	_server->send(id, msg);
}


const bool IPlayerManager::isServerActive() const {
	if (_server == NULL || !_server->active())
		return false;
	
	int n = _players.size();
	for(int i = 0; i < n; ++i) {
		const PlayerSlot &slot = _players[i];
		if (slot.remote && !slot.reserved && slot.id != -1)
			return true;
	}
	return false;
}

void IPlayerManager::onPlayerDeath(const Object *player, const Object *killer) {
	if (isClient())
		return;
	{
		PlayerSlot *player_slot = getSlotByID(player->getID());
		if (player_slot == NULL)
			return;
	}
	PlayerSlot *slot = NULL;

	std::deque<int> owners;
	killer->getOwners(owners);
	for(std::deque<int>::const_iterator i = owners.begin(); i != owners.end(); ++i) {
		slot = getSlotByID(*i);
		if (slot != NULL) 
			break;
	}
	
	if (slot == NULL)
		slot = getSlotByID(killer->getSummoner());

	if (slot == NULL)
		return;
	
	if (killer->getID() == slot->id) 
		return; //skip attachVehicle() call. magic. :)
	
	LOG_DEBUG(("player: %s killed by %s", player->registered_name.c_str(), killer->registered_name.c_str()));
		
	if (slot->id == player->getID()) { //suicide
		if (slot->frags > 0)
			--(slot->frags);
	} else {
		++(slot->frags);
	}
}

void IPlayerManager::getDefaultVehicle(std::string &vehicle, std::string &animation) {
	std::string rv, ra;
	Config->get("multiplayer.restrict-start-vehicle", rv, "");
	Config->get("multiplayer.restrict-start-animation", ra, "");
	if (rv.empty()) {
		if (vehicle.empty()) 
			Config->get("menu.default-vehicle-1", vehicle, "launcher");
	} else vehicle = rv;
	
	if (ra.empty()) {
		if (animation.empty()) {
			if (vehicle == "tank" || vehicle == "launcher" || vehicle == "shilka") {
				static const char * colors[4] = {"green", "red", "yellow", "cyan"};
				animation = colors[mrt::random(4)];
				animation += "-" + vehicle;
			} else animation = vehicle;
		}
	} else animation = ra;
}

void IPlayerManager::gameOver(const std::string &reason, const float time) {
	if (!isServerActive())
		return;
	Message m(Message::GameOver);
	m.set("message", reason);
	m.set("duration", mrt::formatString("%g", time));
	broadcast(m);
}

void IPlayerManager::onDestroyMap(const std::set<v3<int> > & cells) {
	if (!_server)
		return;
		
	mrt::Serializator s;
	s.add((int)cells.size());
	for(std::set<v3<int> >::const_iterator i = cells.begin(); i != cells.end(); ++i) {
		i->serialize(s);
	}

	Message m(Message::DestroyMap);
	m.data = s.getData();
	broadcast(m);	
}

void IPlayerManager::updateControls() {
	int n = _players.size();
	int pn = 0;
	int p1 = -1, p2 = -1;
	
	for(int i = 0; i < n; ++i) {
		const PlayerSlot &slot = _players[i];
		if (slot.visible) {
			++pn;
			if (p1 == -1) {
				p1 = i;
				continue;
			}
			if (p2 == -1) {
				p2 = i;
				continue;
			}
		}
	}
	//LOG_DEBUG(("p1: %d, p2: %d", p1, p2));
	std::string cm1, cm2;
	switch(pn) {
	case 2: 
		Config->get("player.control-method-1", cm1, "keys-1");	
		Config->get("player.control-method-2", cm2, "keys-2");	
		_players[p1].createControlMethod(cm1);
		_players[p2].createControlMethod(cm2);
	break;
	case 1: 
		Config->get("player.control-method", cm1, "keys");	
		_players[p1].createControlMethod(cm1);
	break;	
	}
}

void IPlayerManager::say(const std::string &message) {
TRY {
	LOG_DEBUG(("say('%s')", message.c_str()));

	Message m(Message::PlayerMessage);
	m.set("text", message);
	
	if (_server) {
		PlayerSlot *my_slot = getMySlot();
		if (my_slot == NULL) 
			throw_ex(("cannot get my slot."));
		m.set("nick", my_slot->name);
		Game->getChat()->addMessage(my_slot->name, message);
		broadcast(m);
	}
	if (_client) {
		_client->send(m);
	}
} CATCH("say", {
		Game->clear();
		GameMonitor->displayMessage("errors", "multiplayer-exception", 1);
})
}
