#ifndef BTANKS_OBJECT_GRID_H__
#define BTANKS_OBJECT_GRID_H__

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

#include "math/v2.h"
#include "math/binary.h"
#include <map>
#include <set>
#include <string>
#include <vector>

class Grid {
public: 
	Grid();
	void setSize(const v2<int> &size, const int step);

	void clear();
	void update(const int id, const v2<int> &pos, const v2<int> &size);
	void remove(const int id);

	void collide(std::set<int> &objects, const v2<int>& area_pos, const v2<int>& area_size) const;
	
private:
	typedef std::set<int> IDSet;
	typedef std::vector<IDSet> SetVector;
	typedef std::vector<SetVector> GridMatrix;

	struct Object {
		Object(const v2<int>& pos, const v2<int>& size) : pos(pos), size(size) {}
		v2<int> pos, size;
	};

	void collide(std::set<int> &objects, const GridMatrix &grid, const v2<int> &grid_size, const v2<int>& area_pos, const v2<int>& area_size) const;
	void removeFromGrid(GridMatrix &grid, const v2<int> &grid_size, const int id, const Object &o);
	void update(GridMatrix &grid, const v2<int> &grid_size, const int id, const v2<int> &pos, const v2<int> &size);
	void resize(GridMatrix &grid, const v2<int> &grid_size, const v2<int> &map_size);

	v2<int> _grid_size; 
	v2<int> _grid4_size;
	
	GridMatrix _grid, _grid4;
	
	typedef std::map<const int, Object> Index;
	Index _index;
};

#endif

