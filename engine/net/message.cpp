
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

#include "message.h"
#include "mrt/exception.h"
#include "mrt/chunk.h"
#include "mrt/gzip.h"
#include "mrt/serializator.h"

#include "mrt/tcp_socket.h"
#include <string.h>

#ifdef _WINDOWS
#	include <winsock2.h>
#else
#	include <arpa/inet.h>
#endif

#include <SDL.h>

Message::Message() : channel(-1), type(None), data(), _attrs(), timestamp(SDL_GetTicks()) {}

Message::Message(const Message::Type type) : channel(-1), type(type), data(), _attrs(), timestamp(SDL_GetTicks()) {}

const char * Message::getType() const {
	switch(type) {
	case None: return "None";
	case Ping: return "Ping";
	case Pang: return "Pang";
	case Pong: return "Pong";
	case RequestServerStatus: return "RequestServerStatus";
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
	case RequestObjects: return "RequestObjects";
	case JoinTeam: return "JoinTeam";
	case ServerDiscovery: return "ServerDiscovery";
	case ServerError: return "ServerError";
	}
	return "Unknown/Damaged";
}


void Message::serialize(mrt::Serializator &s) const {
	s.add(channel);
	s.add((int)type);
	s.add<std::string, std::string>(_attrs);
	s.add(data);
	s.add(timestamp);
}

void Message::deserialize(const mrt::Serializator &s) {
	s.get(channel);
	int t;
	s.get(t);
	type = (Message::Type) t;
	s.get<std::string, std::string>(_attrs);
	s.get(data);
	s.get(timestamp);
}

void Message::set(const std::string &key, const std::string &value) {
	_attrs[key] = value;
}

const bool Message::has(const std::string &key) const {
	return _attrs.find(key) != _attrs.end();
}

const std::string &Message::get(const std::string &key) const {
	AttrMap::const_iterator i = _attrs.find(key);
	if (i == _attrs.end())
		throw_ex(("no attribute '%s' found", key.c_str()));
	return i->second;
}

