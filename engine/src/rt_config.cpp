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

#include "rt_config.h"
#include "mrt/serializator.h"

IMPLEMENT_SINGLETON(RTConfig, IRTConfig);

IRTConfig::IRTConfig() : 
	server_mode(false), editor_mode(false), game_type(GameTypeDeathMatch), teams(0), time_limit(0), port(27255),  
	release_name("longcat"), disable_donations(false) {}

void IRTConfig::serialize(mrt::Serializator &s) const {
	s.add((int)game_type);
	s.add(teams);
}

#include "mrt/logger.h"
#include "mrt/exception.h"

void IRTConfig::deserialize(const mrt::Serializator &s) {
	int t;
	s.get(t);
	LOG_DEBUG(("deserialized game type %d", t));
	game_type = (GameType)t;
	s.get(teams);
	LOG_DEBUG(("deserialized teams %d", teams));
}

GameType IRTConfig::parse_game_type(const std::string &type) {
	if (type == "deathmatch") {
		return GameTypeDeathMatch;
	} else if (type == "team-deathmatch") {
		return GameTypeTeamDeathMatch;
	} else if (type == "cooperative") {
		return GameTypeCooperative;
	} else if (type == "racing") {
		return GameTypeRacing;
	} else if (type == "ctf") {
		return GameTypeCTF;
	} else 
		throw_ex(("unsupported game type '%s'", type.c_str()));	
}
