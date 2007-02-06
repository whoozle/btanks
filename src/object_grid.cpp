#include "object_grid.h"
#include "mrt/fmt.h"
#include "mrt/logger.h"
#include <algorithm>
#include "math/binary.h"
#include "config.h"

Grid::Grid() {}

void Grid::clear() {
	_grid.clear();
	_grid4.clear();
	_index.clear();
}

//#define NEW_COLLIDE
#define NEW_OLD_COLLIDE

void Grid::collide(std::set<int> &objects, const GridMap &grid, const v2<int> &grid_size, const v2<int>& area_pos, const v2<int>& area_size) const {
	v2<int> start = area_pos / grid_size;
	v2<int> end = (area_pos + area_size - 1) / grid_size;
	//static int hit = 0, n = 0, global = 0;

#ifndef NEW_COLLIDE
#ifdef NEW_OLD_COLLIDE

	for(int y = start.y; y <= end.y; ++y) {
		GridMap::const_iterator i = grid.find(v2<int>(start.x, y));

		for(int x = start.x; i != grid.end() && i.first.y == y && x <= end.x; ++x) {
			const v2<int> &i_pos = i->first;
			if (i_pos.x > x) {
				x = i_pos.x;
				continue;
			}
			assert(x == i_pos.x);
			
			//++n;
			//++hit;
			std::set_union(objects.begin(), objects.end(), i->second.begin(), i->second.end(), 
				std::insert_iterator<std::set<int> > (objects, objects.begin()));
			++i;
		}	
	}
#else
	//the very simple and straightforward collide.
	for(int y = start.y; y <= end.y; ++y) 
		for(int x = start.x; x <= end.x; ++x) {
//			++n;
			GridMap::const_iterator i = grid.find(v2<int>(x, y));
			if (i == grid.end())
				continue;
//			++hit;
			std::set_union(objects.begin(), objects.end(), i->second.begin(), i->second.end(), 
				std::insert_iterator<std::set<int> > (objects, objects.begin()));
		}	

#endif

#else 
	//no rb-tree search everytime. (actually 2 times per call) 
	//O(area_size.y * size.x)
	GridMap::const_iterator b = grid.lower_bound(start);
	GridMap::const_iterator e = grid.upper_bound(end);
	for(GridMap::const_iterator i = b; i != e; ++i) {
		const v2<int> &pos = i->first;
		if (pos.x >= start.x && pos.x <= end.x && pos.y >= start.y && pos.y <= end.y) {
//			++hit;
			std::set_union(objects.begin(), objects.end(), i->second.begin(), i->second.end(), 
				std::insert_iterator<std::set<int> > (objects, objects.begin()));
		}
//		++n;
	}
#endif
//	++global;
//	LOG_DEBUG(("%d of %d = %g (of %d) avg:  %g of %g", hit, n, 1.0 * hit / n, global, 1.0 * hit / global, 1.0 * n / global));
}

void Grid::collide(std::set<int> &objects, const v2<int>& area_pos, const v2<int>& area_size) const {
	v2<int> size = area_size / _grid_size;
	int n = size.x * size.y;
	GET_CONFIG_VALUE("engine.grid-1x-limit", int, limit, 16);
	if (n >= limit) {
		collide(objects, _grid4, _grid4_size, area_pos, area_size);	
	} else {
		collide(objects, _grid, _grid_size, area_pos, area_size);		
	}
}


void Grid::setSize(const v2<int> &size, const int step) {
	clear();
	_grid_size = v2<int>(step, step);
	_grid4_size = v2<int>(step * 4, step * 4);
}

void Grid::removeFromGrid(GridMap &grid, const v2<int> &grid_size, const int id, const Object &o) {
	v2<int> start = o.pos / grid_size;
	v2<int> end = (o.pos + o.size - 1) / grid_size;
	for(int y = start.y; y <= end.y; ++y) 
		for(int x = start.x; x <= end.x; ++x) {
			GridMap::iterator i = grid.find(v2<int>(x, y));
			if (i == grid.end())
				continue;
			i->second.erase(id);
		}
}

void Grid::update(GridMap &grid, const v2<int> &grid_size, const int id, const v2<int> &pos, const v2<int> &size) {
		//insert
		v2<int> start = pos / grid_size;
		v2<int> end = (pos + size - 1) / grid_size;
		//LOG_DEBUG(("updating %d (%d, %d) -> (%d, %d) (%d %d)", id, start.x, start.y, end.x, end.y, pos.x, pos.y));
		for(int y = start.y; y <= end.y; ++y) 
			for(int x = start.x; x <= end.x; ++x) {
				grid[v2<int>(x, y)].insert(id);
			}

}


void Grid::update(const int id, const v2<int> &pos, const v2<int> &size) {
	Index::iterator i = _index.find(id);
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
		_index.insert(Index::value_type(id, Object(pos, size)));

	update(_grid, _grid_size, id, pos, size);
	update(_grid4, _grid4_size, id, pos, size);
}

void Grid::remove(const int id) {
	Index::iterator i = _index.find(id);
	if (i != _index.end()) {
		removeFromGrid(_grid, _grid_size, id, i->second);		
		removeFromGrid(_grid4, _grid4_size, id, i->second);		
		_index.erase(i);
	} 
}
