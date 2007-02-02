#include "object_grid.h"
#include "mrt/fmt.h"
#include "mrt/logger.h"
#include <algorithm>

Grid::Grid() : _step(0) {}

void Grid::clear() {
	_grid.clear();
	_index.clear();
	_grid_size.clear();
}

void Grid::collide(std::set<int> &objects, const int id) const {
	Index::const_iterator i = _index.find(id);
	if (i == _index.end())
		throw_ex(("nothing known about id %d", id));
	collide(objects, i->second.pos, i->second.size);
}

void Grid::collide(std::set<int> &objects, const v2<int>& area_pos, const v2<int>& area_size) const {
	v2<int> start = area_pos / _grid_size;
	v2<int> end = (area_pos + area_size - 1) / _grid_size;
	for(int y = start.y; y <= end.y; ++y) 
		for(int x = start.x; x <= end.x; ++x) {
			GridMap::const_iterator i = _grid.find(v2<int>(x, y));
			if (i == _grid.end())
				continue;

			std::set_union(objects.begin(), objects.end(), i->second.begin(), i->second.end(), 
				std::insert_iterator<std::set<int> > (objects, objects.begin()));
		}	
}

void Grid::setSize(const v2<int> &size, const int step) {
	clear();
	_grid_size = (size - 1) / step + 1;
	//LOG_DEBUG(("grid_size: %dx%d", _grid_size.x, _grid_size.y));
	_step = step;
}

void Grid::removeFromGrid(const int id, const Object &o) {
	v2<int> start = o.pos / _grid_size;
	v2<int> end = (o.pos + o.size - 1) / _grid_size;
	for(int y = start.y; y <= end.y; ++y) 
		for(int x = start.x; x <= end.x; ++x) {
			GridMap::iterator i = _grid.find(v2<int>(x, y));
			if (i == _grid.end())
				continue;
			i->second.erase(id);
		}
}

void Grid::update(const int id, const v2<int> &pos, const v2<int> &size) {
	Index::iterator i = _index.find(id);
	if (i != _index.end()) {
		removeFromGrid(id, i->second);		
		i->second.pos = pos;
		i->second.size = size;
	} else 
		_index.insert(Index::value_type(id, Object(pos, size)));

	{
		//insert
		v2<int> start = pos / _grid_size;
		v2<int> end = (pos + size - 1) / _grid_size;
		//LOG_DEBUG(("updating %d (%d, %d) -> (%d, %d) (%d %d)", id, start.x, start.y, end.x, end.y, pos.x, pos.y));
		for(int y = start.y; y <= end.y; ++y) 
			for(int x = start.x; x <= end.x; ++x) {
				_grid[v2<int>(x, y)].insert(id);
			}
	}
}

void Grid::remove(const int id) {
	Index::iterator i = _index.find(id);
	if (i != _index.end()) {
		removeFromGrid(id, i->second);		
		_index.erase(i);
	} 
}
