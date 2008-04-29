#ifndef __BTANKS_RT_CONFIG_H__
#define __BTANKS_RT_CONFIG_H__

/* Battle Tanks Game
 * Copyright (C) 2006-2008 Battle Tanks team
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

enum GameType {
	GameTypeDeathMatch, GameTypeCooperative, GameTypeRacing, GameTypeCTF, GameTypeTeamDeathMatch
};

class BTANKSAPI IRTConfig {
public:
	DECLARE_SINGLETON(IRTConfig);
	IRTConfig();
	
	bool server_mode;
	GameType game_type;
	int teams; //for team games, usually 2 for CTF

	void serialize(Serializator &s) const;
	void deserialize(const Serializator &s);
};

SINGLETON(BTANKSAPI, RTConfig, IRTConfig);

#endif

