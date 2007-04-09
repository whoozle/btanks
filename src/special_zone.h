#ifndef BTANKS_SPECIAL_ZONE_H__
#define BTANKS_SPECIAL_ZONE_H__

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

#include "zbox.h"
#include <string>

class SpecialZone : public ZBox {
public: 
	std::string area, type, name, subname;

	SpecialZone(const ZBox & zbox, const std::string &type, const std::string &name, const std::string &subname);

	void onEnter(const int slot_id);

	const bool final() const;
private: 
	void onCheckpoint(const int slot_id);
	void onHint(const int slot_id);
	void onMessage(const int slot_id);
	void onTimer(const int slot_id, const bool win);
};

#endif

