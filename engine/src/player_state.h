#ifndef __BTANKS_PLAYER_STATE_H__
#define __BTANKS_PLAYER_STATE_H__

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

/* 
 * Additional rights can be granted beyond the GNU General Public License 
 * on the terms provided in the Exception. If you modify this file, 
 * you may extend this exception to your version of the file, 
 * but you are not obligated to do so. If you do not wish to provide this
 * exception without modification, you must delete this exception statement
 * from your version and license this file solely under the GPL without exception. 
*/


#include "export_btanks.h"
#include "mrt/serializable.h"
#include "math/v2.h"
#include <string>

class BTANKSAPI PlayerState : public mrt::Serializable {
public:
	unsigned left: 1;
	unsigned right: 1;
	unsigned up:1;
	unsigned down:1;
	unsigned fire:1;
	unsigned alt_fire:1;
	unsigned leave:1;
	unsigned hint_control:1;

	PlayerState();
	void clear();
	
	const bool operator==(const PlayerState &other) const;
	inline const bool operator!=(const PlayerState &other) const {
		return !(*this == other);
	}
	bool compare_directions(const PlayerState &other) const;

	template <typename T>
	void get_velocity(v2<T> &result) {
		result.clear();
		if (left)
			--result.x;
		if (right)
			++result.x;
		if (up)
			--result.y;
		if (down)
			++result.y;
	}

	virtual void serialize(mrt::Serializator &s) const;
	virtual void deserialize(const mrt::Serializator &s);
	
	const std::string dump() const;
};

#endif

