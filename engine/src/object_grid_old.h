#ifndef BTANKS_OBJECT_GRID_H__
#define BTANKS_OBJECT_GRID_H__

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

#include <map>
#include <string>
#include <set>
#include <algorithm>
#include "math/v2.h"
#include "math/binary.h"
#include "mrt/hash_compat.h"

template<typename T> 
class Grid {
public: 
	Grid() {}

	void set_size(const v2<int> &size, const int step, const bool wrap) {
		clear();
		_grid_size = v2<int>(step, step);
		resize(_grid, _grid_size, size);
		_grid4_size = v2<int>(step * 4, step * 4);
		resize(_grid4, _grid4_size, size);
		_wrap = wrap;
		_map_size = size;
	}

	void clear() {
		_grid.clear();
		_grid4.clear();
		_index.clear();
	}

	void update(T id, const v2<int> &pos, const v2<int> &size) {
		if (_grid.empty())
			return;
		
		typename Index::iterator i = _index.find(id);
		if (i != _index.end()) {
		//skip modification if grid coordinates
			if (pos / _grid_size == i->second.pos / _grid_size &&
				(pos + size - 1) / _grid_size == (i->second.pos + size - 1) / _grid_size) {
				return;	
			}
			

			removeFromGrid(_grid, _grid_size, id, i->second);
			removeFromGrid(_grid4, _grid4_size, id, i->second);
			i->second.pos = pos;
			i->second.size = size;
		} else 
			_index.insert(typename Index::value_type(id, Object(pos, size)));

		update(_grid, _grid_size, id, pos, size);
		update(_grid4, _grid4_size, id, pos, size);
	}

	void remove(T id) {
		if (_grid.empty())
			return;

		typename Index::iterator i = _index.find(id);
		if (i != _index.end()) {
			removeFromGrid(_grid, _grid_size, id, i->second);		
			removeFromGrid(_grid4, _grid4_size, id, i->second);		
			_index.erase(i);
		} 
	}

	void collide(std::set<T> &objects, const v2<int>& area_pos, const v2<int>& area_size) const {
		if (_grid.empty())
			return;

		v2<int> size = (area_size - 1) / _grid_size + 1;
		int n = size.x * size.y;
		if (n >= 16) { //replace with config ? 
			collide(objects, _grid4, _grid4_size, area_pos, area_size);	
		} else {
			collide(objects, _grid, _grid_size, area_pos, area_size);		
		}
		//LOG_DEBUG(("returned %u objects", (unsigned)objects.size()));
	}
	
private:
	static inline int wrap(int x, int y) {
		x %= y;
		return x < 0? x + y: x;
	}

	typedef std::set<T> IDSet;
	typedef std::vector<IDSet> SetVector;
	typedef std::vector<SetVector> GridMatrix;

	struct Object {
		Object(const v2<int>& pos, const v2<int>& size) : pos(pos), size(size) {}
		v2<int> pos, size;
	};

	void collide(std::set<T> &result, const GridMatrix &grid, const v2<int> &grid_size, const v2<int>& area_pos, const v2<int>& area_size) const {
		v2<int> delta = v2<int>(grid[0].size(), grid.size()) * grid_size - _map_size;
		
		const v2<int> start = area_pos / grid_size;
		v2<int> end = (area_pos + area_size - 1) / grid_size;

		if (end.y < (int)grid.size() - 1) delta.y = 0;
		if (end.x < (int)grid[0].size() - 1) delta.x = 0;
		end = (area_pos + area_size + delta - 1) / grid_size;

		const int y1 = _wrap? start.y: math::max(0, start.y);
		const int y2 = _wrap? end.y: math::min((int)grid.size() - 1, end.y);
		const int x1 = _wrap? start.x: math::max(0, start.x);
		for(int y = y1; y <= y2; ++y) {
			const SetVector &row = grid[wrap(y, grid.size())];
			const int x2 = _wrap?end.x: math::min((int)row.size() - 1, end.x);
			for(int x = x1; x <= x2; ++x) {
				const IDSet &set = row[wrap(x, row.size())];
				result.insert(set.begin(), set.end());
			}
		}
	}

	void removeFromGrid(GridMatrix &grid, const v2<int> &grid_size, T id, const Object &o) {
		v2<int> delta = v2<int>(grid[0].size(), grid.size()) * grid_size - _map_size;

		const v2<int> start = o.pos / grid_size;
		v2<int> end = (o.pos + o.size - 1) / grid_size;

		if (end.y < (int)grid.size() - 1) delta.y = 0;
		if (end.x < (int)grid[0].size() - 1) delta.x = 0;
		end = (o.pos + o.size + delta - 1) / grid_size;

		const int y1 = _wrap? start.y: math::max(0, start.y);
		const int y2 = _wrap? end.y: math::min((int)grid.size() - 1, end.y);
		const int x1 = _wrap? start.x: math::max(0, start.x);
		for(int y = y1; y <= y2; ++y) {
			SetVector &row = grid[wrap(y, grid.size())];
			const int x2 = _wrap? end.x: math::min((int)row.size() - 1, end.x);
			for(int x = x1; x <= x2; ++x) {
				row[wrap(x, row.size())].erase(id);
			}
		}
	}

	void update(GridMatrix &grid, const v2<int> &grid_size, T id, const v2<int> &pos, const v2<int> &size) {
		v2<int> delta = v2<int>(grid[0].size(), grid.size()) * grid_size - _map_size;
		
		const v2<int> start = pos / grid_size;
		v2<int> end = (pos + size - 1) / grid_size;
		
		if (end.y < (int)grid.size() - 1) delta.y = 0;
		if (end.x < (int)grid[0].size() - 1) delta.x = 0;
		end = (pos + size + delta - 1) / grid_size;
		
		//LOG_DEBUG(("updating %d (%d, %d) -> (%d, %d) (%d %d)", id, start.x, start.y, end.x, end.y, pos.x, pos.y));
		
		const int y1 = _wrap? start.y: math::max(0, start.y);
		const int y2 = _wrap? end.y: math::min((int)grid.size() - 1, end.y);
		const int x1 = _wrap? start.x: math::max(0, start.x);
		for(int y = y1; y <= y2; ++y) {
			SetVector &row = grid[wrap(y, grid.size())];
			const int x2 = _wrap? end.x: math::min((int)grid[y].size() - 1, end.x);
			for(int x = x1; x <= x2; ++x) {
				row[wrap(x, row.size())].insert(id);
			}
		}
	}

	void resize(GridMatrix &grid, const v2<int> &grid_size, const v2<int> &map_size) {
		v2<int> dim = (map_size - 1) / grid_size + 1;
		grid.resize(dim.y);
		for(int y = 0; y < dim.y; ++y) 
			grid[y].resize(dim.x);
	}

	v2<int> _grid_size; 
	v2<int> _grid4_size;
	
	v2<int> _map_size;
	
	GridMatrix _grid, _grid4;
	
	struct object_hash {
		inline size_t operator()(const ::Object *o) const { 
			size_t x = (size_t)o;
			size_t r = 0x1b766561;
			for(int i = 0; i < 4; ++i) {
				size_t c = (x >> (8 * i)) & 0xff;
				r ^= ((r << 5) + c + (r >> 2));
			}
			return r;
		}
	};
	
	typedef MRT_HASH_MAP <T, Object, object_hash > Index;
	
	Index _index;
	bool _wrap;
};

#endif

