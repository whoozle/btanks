
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

#include "protocol.h"
#include "mrt/exception.h"
#include "mrt/chunk.h"
#include "mrt/gzip.h"
#include "mrt/serializator.h"

#include "mrt/tcp_socket.h"
#include <string.h>

#ifdef WIN32
#	include <winsock2.h>
#else
#	include <arpa/inet.h>
#endif

Message::Message() : channel(0), type(None), data(), _attrs() {}

Message::Message(const Message::Type type) : channel(0), type(type), data(), _attrs() {}

const char * Message::getType() const {
	switch(type) {
	case None: return "None";
	case Ping: return "Ping";
	case Pang: return "Pang";
	case Pong: return "Pong";
	case ServerStatus: return "ServerStatus";
	case RequestPlayer: return "RequestPlayer";
	case GameJoined: return "GameJoined";
	case PlayerState: return "PlayerState";
	case UpdatePlayers: return "UpdatePlayers";
	case UpdateWorld: return "UpdateWorld";
	case Respawn: return "Respawn";
	case GameOver: return "GameOver";
	case TextMessage: return "TextMessage";
	case DestroyMap: return "DestroyMap";
	case PlayerMessage: return "PlayerMessage";
	}
	return "Unknown/Damaged";
}


void Message::serialize(mrt::Serializator &s) const {
	s.add(channel);
	s.add((int)type);
	writeMap(s);
	s.add(data);
}

void Message::deserialize(const mrt::Serializator &s) {
	s.get(channel);
	int t;
	s.get(t);
	type = (Message::Type) t;
	readMap(s);
	s.get(data);
}

void Message::writeMap(mrt::Serializator &s) const {
	int size = _attrs.size();
	s.add(size);
	for(AttrMap::const_iterator ai = _attrs.begin(); ai != _attrs.end(); ++ai) {
		s.add(ai->first);
		s.add(ai->second);
	}
}


void Message::readMap(const mrt::Serializator &s) {
	_attrs.clear();
	
	int n;
	s.get(n);

	std::string key;
	std::string value;
	
	while(n--) {
		s.get(key);
		s.get(value);
		_attrs[key] = value;
	}
}


void Message::set(const std::string &key, const std::string &value) {
	_attrs[key] = value;
}

const std::string &Message::get(const std::string &key) const {
	AttrMap::const_iterator i = _attrs.find(key);
	if (i == _attrs.end())
		throw_ex(("no attribute '%s' found", key.c_str()));
	return i->second;
}
