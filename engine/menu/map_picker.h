#ifndef BTANKS_MAP_PICKER_H__
#define BTANKS_MAP_PICKER_H__

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

#include "container.h"
#include <vector>
#include <string>
#include <map>
#include "map_desc.h"

class ScrollList;
class MapDetails;
class PlayerPicker;
class UpperBox;
class ModePanel;
class Notepad;

class MapPicker : public Container {
public: 

	MapPicker(const int w, const int h);
	virtual void tick(const float dt);
	
	const MapDesc &getCurrentMap() const;
	void fillSlots() const;

	void getBaseSize(int &y1, int &y2) { y1 = ybase1; y2 = ybase2; }
	
private:
	void reload(); 
	void loadScreenshot();
	void scan(const std::string &dir);

	int _index;
	typedef std::vector<MapDesc > MapList;
	MapList _maps;
	
	UpperBox * _upper_box;
	ScrollList *_list;
	MapDetails *_details;
	PlayerPicker *_picker;
	ModePanel *_mode_panel;
	Notepad *notepad;
	
	std::map<const int, int> map_indexes;
	
	int ybase1, ybase2;
};

#endif

