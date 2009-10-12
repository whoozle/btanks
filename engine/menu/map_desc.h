#ifndef BTANKS_MENU_MAP_DESC_H__
#define BTANKS_MENU_MAP_DESC_H__

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

#include <string>
#include "rt_config.h"

	struct MapDesc {
		std::string base, name, object_restriction;
		GameType game_type;
		int slots;
		bool supports_ctf;
		
		MapDesc(const std::string &base, const std::string &name, const std::string &object, const GameType game_type, const int slots, bool supports_ctf) : 
			base(base), name(name), object_restriction(object), game_type(game_type), slots(slots), supports_ctf(supports_ctf) {}

		const bool operator<(const MapDesc &other) const;
	};


#endif
