#ifndef __BTANKS_PROTOCOL_H__
#define __BTANKS_PROTOCOL_H__

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


#include <sys/types.h>
#include <map>
#include <string>
#include "mrt/serializable.h"
#include "mrt/chunk.h"

namespace mrt {
	class TCPSocket;
}

	
class Message : public mrt::Serializable {
public: 
	enum Type {
		None, Ping, Pang, Pong,
		ServerStatus,
		RequestPlayer,
		GameJoined,
		PlayerState,
		UpdatePlayers,
		UpdateWorld, 
		Respawn, 
		GameOver,
		TextMessage, 
		DestroyMap
	};
	
	Message();
	Message(const Type type);
	
	const char * getType() const;

	virtual void serialize(mrt::Serializator &s) const;
	virtual void deserialize(const mrt::Serializator &s);
	
	void set(const std::string &key, const std::string &value);
	const std::string &get(const std::string &key) const;
	
	Type type;

	mrt::Chunk data;
private:
	void readMap(const mrt::Serializator &s);
	void writeMap(mrt::Serializator &s) const;

	typedef std::map<const std::string, std::string> AttrMap;
	AttrMap _attrs;
};

#endif

