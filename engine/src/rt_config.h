#ifndef __BTANKS_RT_CONFIG_H__
#define __BTANKS_RT_CONFIG_H__

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


#include "mrt/singleton.h"
#include "game_type.h"
#include "export_btanks.h"
#include <string>

namespace mrt {
	class Serializator;
}

class BTANKSAPI IRTConfig {
public:
	DECLARE_SINGLETON(IRTConfig);
	IRTConfig();
	
	bool server_mode, editor_mode;
	GameType game_type;
	int teams; //for team games, usually 2 for CTF

	float time_limit;
	int port;
	
	const std::string release_name;
	
	bool disable_donations, disable_network;

	void serialize(mrt::Serializator &s) const;
	void deserialize(const mrt::Serializator &s);
	
	static GameType parse_game_type(const std::string &type);
};

PUBLIC_SINGLETON(BTANKSAPI, RTConfig, IRTConfig);

#endif

