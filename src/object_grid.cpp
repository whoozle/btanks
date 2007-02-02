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

void Grid::collide(std::set<int> &objects, const GridMap &grid, const v2<int> &grid_size, const v2<int>& area_pos, const v2<int>& area_size) const {
	v2<int> start = area_pos / grid_size;
	v2<int> end = (area_pos + area_size - 1) / grid_size;
	for(int y = start.y; y <= end.y; ++y) 
		for(int x = start.x; x <= end.x; ++x) {
			GridMap::const_iterator i = grid.find(v2<int>(x, y));
			if (i == grid.end())
				continue;

			std::set_union(objects.begin(), objects.end(), i->second.begin(), i->second.end(), 
				std::insert_iterator<std::set<int> > (objects, objects.begin()));
		}	
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
