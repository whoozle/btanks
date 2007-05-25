#ifndef BTANKS_MAP_PICKER_H__
#define BTANKS_MAP_PICKER_H__

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

#include "container.h"
#include <vector>
#include <string>
#include "map_desc.h"

class ScrollList;
class MapDetails;
class PlayerPicker;
class UpperBox;

class MapPicker : public Container {
public: 

	MapPicker(const int w, const int h);
	virtual void tick(const float dt);
	
	const MapDesc &getCurrentMap() const { return _maps[_index]; }
	void fillSlots() const;

private:
	void loadScreenshot();
	void scan(const std::string &dir);

	int _index;
	typedef std::vector<MapDesc > MapList;
	MapList _maps;
	
	UpperBox * _upper_box;
	ScrollList *_list;
	MapDetails *_details;
	PlayerPicker *_picker;
};

#endif

