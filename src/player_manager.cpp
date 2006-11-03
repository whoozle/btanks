
/* Battle Tanks Game
 * Copyright (C) 2006 Battle Tanks team
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

#include "controls/keyplayer.h"
#include "controls/joyplayer.h"
#include "controls/mouse_control.h"
//#include "controls/external_control.h"

#include "net/server.h"
#include "net/client.h"
#include "net/protocol.h"
#include "version.h"

#include "tmx/map.h"
#include "sound/mixer.h"

#include "sdlx/surface.h"
#include "game.h"

#include "mrt/random.h"

IMPLEMENT_SINGLETON(PlayerManager, IPlayerManager)


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
	message.set("map", Map->getName());
	message.set("version", getVersion());

	mrt::Serializator s;
	//World->serialize(s);
	s.add(client_id);
	//_players[client_id].position.serialize(s);
	
	serializeSlots(s);	

	message.data = s.getData();
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
		obj->emit("death", NULL);
	
	slot.clear();
}


void IPlayerManager::onMessage(const int id, const Message &message) {
TRY {
	LOG_DEBUG(("incoming message %s", message.getType()));
	switch(message.type) {
	case Message::ServerStatus: {
		LOG_DEBUG(("server version: %s", message.get("version").c_str()));
		LOG_DEBUG(("loading map..."));
		Game->loadMap(message.get("map"), false);
		
		mrt::Serializator s(&message.data);
		//World->deserialize(s);
		
		s.get(_my_idx);
		LOG_DEBUG(("my_idx = %d", _my_idx));
		Game->setMyIndex(_my_idx);
		
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
		createControlMethod(slot, control_method);

		LOG_DEBUG(("players = %d", _players.size()));
		
		Message m(Message::RequestPlayer);

		GET_CONFIG_VALUE("stubs.default-mp-vehicle", std::string, vehicle, "tank");

		m.set("vehicle", vehicle);
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
		
		const std::string &vehicle = message.get("vehicle");
		
		static const char * colors[4] = {"green", "red", "yellow", "cyan"};
		std::string animation = colors[mrt::random(4)];
		animation += "-" + vehicle;

		LOG_DEBUG(("player%d: %s:%s", id, vehicle.c_str(), animation.c_str()));
		spawnPlayer(slot, vehicle, animation);

		slot.reserved = false;
		slot.remote = true;

		mrt::Serializator s;
		World->serialize(s);
		serializeSlots(s);
		
		Message m(Message::GameJoined);
		m.data = s.getData();
		_server->send(id, m);

		break;
	}
	
	case Message::GameJoined: {
		assert(_my_idx >= 0);
		mrt::Serializator s(&message.data);
		World->deserialize(s);
		deserializeSlots(s);		
		break;
	}
	
	case Message::UpdateWorld: {
		mrt::Serializator s(&message.data);
		deserializeSlots(s);
		World->applyUpdate(s, _trip_time / 1000.0);
		break;
	} 
	case Message::PlayerState: {
		mrt::Serializator s(&message.data);
		if (id < 0 || (unsigned)id >= _players.size())
			throw_ex(("player id exceeds players count (%d/%d)", id, _players.size()));
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

		//World->tick(*slot.obj, -slot.trip_time / 1000.0);
		Object * obj = World->deserializeObjectInfo(s, slot.id);
		if (obj == NULL) 
			throw_ex(("player state for non-existent object %d recv'ed", slot.id));
		
		assert(slot.id == obj->getID());
		
		slot.need_sync = obj->updatePlayerState(state);
		if (slot.need_sync == false)
			throw_ex(("player %d send duplicate player state. bye-bye", id));
		
		World->tick(*obj, slot.trip_time / 1000.0);
		break;
	} 
	case Message::UpdatePlayers: { 
		mrt::Serializator s(&message.data);
		std::set<Object *> updated_objects;
		while(!s.end()) {
			int id;
			s.get(id);
			unsigned slot;
			for(slot = 0; slot < _players.size(); ++slot) {
				if (_players[slot].id == id)
					break;
			}
			const bool my_state = slot < _players.size() && slot == (unsigned)_my_idx;
			
			PlayerState state; 
			state.deserialize(s);
			LOG_DEBUG(("slot: %d, id: %d, state: %s %s", slot, id, state.dump().c_str(), my_state?"skipped":""));
			Object *o = World->deserializeObjectInfo(s, id, my_state);
			if (o == NULL) {
				LOG_DEBUG(("still dont know anything about object %d, skipping for now", id));
				continue;
			}
			if (my_state)
				continue;
			
			o->updatePlayerState(state); //update states for all players but me.

			if (slot < _players.size()) {
				_players[slot].id = o->getID(); // ???
			}
			updated_objects.insert(o);
		}	
		World->tick(updated_objects, _trip_time / 1000.0);
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
			throw_ex(("player id exceeds players count (%d/%d)", id, _players.size()));
		float ping = extractPing(message.data);
		
		GET_CONFIG_VALUE("multiplayer.ping-interpolation-multiplier", int, pw, 3);
		_players[id].trip_time = (pw * ping + _players[id].trip_time) / (pw + 1);
		LOG_DEBUG(("player %d: ping: %g ms", id, ping));		
		break;
	}
	
	case Message::Respawn: {
		TRY {
		if (id < 0 || (unsigned)id >= _players.size())
			throw_ex(("player id exceeds players count (%d/%d)", id, _players.size()));
		mrt::Serializator s(&message.data);
		deserializeSlots(s);
		World->applyUpdate(s, _trip_time / 1000.0);
		} CATCH("message::respawn", throw;);
	break;
	}
	default:
		LOG_WARN(("unhandled message: %s\n%s", message.getType(), message.data.dump().c_str()));
	};
} CATCH("onMessage", { 
	if (_server) 
		_server->disconnect(id);
	if (_client) 
		_client->disconnect();
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
	int n = _players.size();

	if (!isClient())
	for(int i = 0; i < n; ++i) {
		PlayerSlot &slot = _players[i];
		if (slot.id <= 0)
			continue;
		
		Object *o = slot.getObject();
		if (o != NULL /* && !o->isDead() */) 
			continue;
			
		if (slot.frags > 0) 
			--slot.frags;

		LOG_DEBUG(("player in slot %d is dead. respawning. frags: %d", i, slot.frags));

		spawnPlayer(slot, slot.classname, slot.animation);
		if (slot.remote) {
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
		if (slot.control_method != NULL) {
			Object *obj = slot.getObject();
			if (obj == NULL && isClient())
				continue;
			
			assert(obj != NULL);
			
			PlayerState old_state = obj->getPlayerState();
			PlayerState state = old_state;
			slot.control_method->updateState(state);
			if (obj->updatePlayerState(state)) {
				updated = true;
				slot.need_sync = true;
			}
			//this will never happen now, as external control method is not used anymore
			/*
			if (slot.need_sync && slot.remote) {
				LOG_DEBUG(("correcting remote player. "));
				obj->getPlayerState() = old_state;
				World->tick(*obj, -slot.trip_time / 1000.0);
				obj->getPlayerState() = state;
				World->tick(*obj, 2 * slot.trip_time / 1000.0);
			}
			*/
		}
		if (slot.need_sync)
			updated = true;
	}
				
	if (_client && _players.size() != 0 && _players[_my_idx].need_sync)	{
		mrt::Serializator s;
		PlayerSlot &slot = _players[_my_idx];
		
		Object * o = World->getObjectByID(slot.id);
		o->getPlayerState().serialize(s);
		World->serializeObjectInfo(s, slot.id);
		
		Message m(Message::PlayerState);
		m.data = s.getData();
		_client->send(m);
		_players[_my_idx].need_sync = false;
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
				Object * o = World->getObjectByID(slot.id);
				assert(o != NULL);
				o->getPlayerState().serialize(s);
				World->serializeObjectInfo(s, slot.id);
				send = true;
				slot.need_sync = false;
			}
		}

		Message m(Message::UpdatePlayers);
		m.data = s.getData();
		
		if (send)
			broadcast(m);
	}
}


const float IPlayerManager::extractPing(const mrt::Chunk &data) const {
	if (data.getSize() < sizeof(unsigned int))
		throw_ex(("invalid pong recv'ed. (size: %u)", data.getSize()));
	
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
	_server(NULL), _client(NULL), _my_idx(-1), _players(), _trip_time(10), _next_ping(0), _ping(false), _next_sync(true) 
{}

IPlayerManager::~IPlayerManager() {}

void IPlayerManager::startServer() {
	delete _client;
	_client = NULL;
	delete _server;
	 
	_server = new Server;
	_server->init(9876);
}

void IPlayerManager::startClient(const std::string &address) {
	delete _client;
	_client = NULL;
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
	delete _server; _server = NULL;
	delete _client; _client = NULL;
	_my_idx = -1;

	GET_CONFIG_VALUE("multiplayer.sync-interval", float, sync_interval, 103.0/101);
	_next_sync.set(sync_interval);

	LOG_DEBUG(("cleaning up players..."));
	_players.clear();	
}

void IPlayerManager::addSlot(const v3<int> &position) {
	PlayerSlot slot;
	slot.position = position;
	_players.push_back(slot);
}

PlayerSlot &IPlayerManager::getSlot(const unsigned int idx) {
	return _players[idx];
}
const PlayerSlot &IPlayerManager::getSlot(const unsigned int idx) const {
	return _players[idx];
}

void IPlayerManager::createControlMethod(PlayerSlot &slot, const std::string &control_method) {
	delete slot.control_method;
	slot.control_method = NULL;

	if (control_method == "keys") {
		slot.control_method = new KeyPlayer(SDLK_UP, SDLK_DOWN, SDLK_LEFT, SDLK_RIGHT, SDLK_LCTRL, SDLK_LALT);
	} else if (control_method == "keys-1") {
		slot.control_method = new KeyPlayer(SDLK_r, SDLK_f, SDLK_d, SDLK_g, SDLK_q, SDLK_a);
	} else if (control_method == "keys-2") {
		slot.control_method = new KeyPlayer(SDLK_UP, SDLK_DOWN, SDLK_LEFT, SDLK_RIGHT, SDLK_RCTRL, SDLK_RSHIFT);
	} else if (control_method == "mouse") {
		throw_ex(("fix mouse control method, then disable this exception ;)"));
		slot.control_method = new MouseControl();
	} else if (control_method == "joy-1") {
		slot.control_method = new JoyPlayer(0, 0, 1);
	} else if (control_method == "joy-2") {
		slot.control_method = new JoyPlayer(1, 0, 1);
	} else if (control_method == "network") {
		//slot.control_method = new ExternalControl;
		slot.control_method = NULL;
		slot.remote = true;
	} else if (control_method != "ai") {
		throw_ex(("unknown control method '%s' used", control_method.c_str()));
	}
}

const int IPlayerManager::findEmptySlot() const {
	int i, n = _players.size();
	for(i = 0; i < n; ++i) {
		if (_players[i].getObject() == NULL && !_players[i].reserved)
			break;
	}
	if (i == n) 
		throw_ex(("no available slots found from %d", n));
	return i;
}

const int IPlayerManager::spawnPlayer(const std::string &classname, const std::string &animation, const std::string &control_method) {
	int i = findEmptySlot();
	PlayerSlot &slot = _players[i];

	createControlMethod(slot, control_method);
	
	LOG_DEBUG(("player: %s.%s using control method: %s", classname.c_str(), animation.c_str(), control_method.c_str()));
	spawnPlayer(slot, classname, animation);
	return i;
}

void IPlayerManager::spawnPlayer(PlayerSlot &slot, const std::string &classname, const std::string &animation) {
	Object *obj = ResourceManager->createObject(classname, animation);
	assert(obj != NULL);

	World->addObject(obj, slot.position.convert<float>(), slot.id);
	Object *spawn = World->spawn(obj, "spawn-shield", "spawning", v3<float>::empty, v3<float>::empty);
	spawn->follow(obj, Centered);

	slot.id = obj->getID();
	slot.classname = classname;
	slot.animation = animation;
}

void IPlayerManager::setViewport(const int idx, const sdlx::Rect &rect) {
	_players[idx].visible = true;
	_players[idx].viewport = rect;
}

void IPlayerManager::tick(const float now, const float dt) {
	if (_server) {
		if (_next_sync.tick(dt) && isServerActive()) {
			Message m(Message::UpdateWorld);
			{
				mrt::Serializator s;
				serializeSlots(s);
				World->generateUpdate(s, true);
				m.data = s.getData();
			}
			LOG_DEBUG(("sending world update... (size: %u)", m.data.getSize()));
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


			for(unsigned int pi = 0; pi < _players.size(); ++pi) {
				PlayerSlot &slot = _players[pi];
				if (!slot.visible)
					continue;
				
				const Object * p = slot.getObject();

				if (p == NULL)
					continue; 
					
				v3<float> pos, vel;
				p->getInfo(pos, vel);

				if ((int)pi == 0)
					Mixer->setListener(pos, vel);
					
				sdlx::Rect passive_viewport;
				passive_viewport.w = passive_viewport.x = slot.viewport.w / 3;
				passive_viewport.h = passive_viewport.y = slot.viewport.h / 3;
				sdlx::Rect passive_viewport_stopzone(passive_viewport);
	
				{
					int xmargin = passive_viewport_stopzone.w / 4;
					int ymargin = passive_viewport_stopzone.h / 4;
					passive_viewport_stopzone.x += xmargin;
					passive_viewport_stopzone.y += ymargin;
					passive_viewport_stopzone.w -= 2*xmargin;
					passive_viewport_stopzone.h -= 2*ymargin;
				}


				//LOG_DEBUG(("player[0] %f, %f", vel.x, vel.y));
				int wx = (int)(pos.x - slot.mapx);
				int wy = (int)(pos.y - slot.mapy);
				if (passive_viewport_stopzone.in(wx, wy)) {
					slot.mapvx = 0; 
					slot.mapvy = 0;
				} else {
					slot.mapvx = p->speed * 2 * (wx - passive_viewport.x) / passive_viewport.w ;
					slot.mapvy = p->speed * 2 * (wy - passive_viewport.y) / passive_viewport.h ;
					/*
					LOG_DEBUG(("position : %f %f viewport: %d %d(passive:%d %d %d %d) mapv: %f %f", x, y,
						viewport.x, viewport.y, passive_viewport.x, passive_viewport.y, passive_viewport.w, passive_viewport.h, 
						mapvx, mapvy));
					*/
				}
			}

		if (Map->loaded()) {
			const v3<int> world_size = Map->getSize();
			for(unsigned p = 0; p < _players.size(); ++p) {
				PlayerSlot &slot = _players[p];
				if (!slot.visible) 
					continue;
				
				slot.mapx += slot.mapvx * dt;
				slot.mapy += slot.mapvy * dt;
			
				if (slot.mapx < 0) 
					slot.mapx = 0;
				if (slot.mapx + slot.viewport.w > world_size.x) 
					slot.mapx = world_size.x - slot.viewport.w;

				if (slot.mapy < 0) 
					slot.mapy = 0;
				if (slot.mapy + slot.viewport.h > world_size.y) 
					slot.mapy = world_size.y - slot.viewport.h;
			
				//LOG_DEBUG(("%f %f", mapx, mapy));
			}
		}

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
	
			World->render(window, sdlx::Rect((int)slot.mapx, (int)slot.mapy, slot.viewport.w, slot.viewport.h),  slot.viewport);
	
			slot.viewport.x -= vx;
			slot.viewport.y -= vy;
		}
}

void IPlayerManager::screen2world(v3<float> &pos, const int p, const int x, const int y) {
	PlayerSlot &slot = _players[p];
	pos.x = slot.mapx + x;
	pos.y = slot.mapx + y;
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

}

void IPlayerManager::deserializeSlots(const mrt::Serializator &s) {
	int n, pn = _players.size();
	s.get(n);
	assert(pn == n);
	
	for(int i = 0; i < n; ++i) {
		_players[i].deserialize(s);
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
