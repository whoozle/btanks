#ifndef BTANKS_SPECIAL_ZONE_H__
#define BTANKS_SPECIAL_ZONE_H__

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

#include "zbox.h"
#include <string>
#include "math/v3.h"

class SpecialZone : public ZBox {
public: 
	std::string area, type, name, subname;

	SpecialZone(const ZBox & zbox, const std::string &type, const std::string &name, const std::string &subname);

	inline const bool global() const { return _global; }
	inline const bool final() const  { return _final; }
	inline const bool live() const   { return _live; }

	void onEnter(const int slot_id);
	void onTick(const int slot_id); //only for 'live' zones
	void onExit(const int slot_id);

	~SpecialZone();
	
	const v3<int> getPlayerPosition(const int slot_id) const;
	
private: 
	void onCheckpoint(const int slot_id);
	void onHint(const int slot_id);
	void on_message(const int slot_id);
	void onTimer(const int slot_id, const bool win);
	void onWarp(const int slot_id, const bool enter);
	
	bool _global, _final, _live;
};

#endif

