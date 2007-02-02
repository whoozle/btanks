#ifndef BTANKS_OBJECT_GRID_H__
#define BTANKS_OBJECT_GRID_H__

#include "math/v2.h"
#include "math/binary.h"
#include <map>
#include <set>
#include <string>

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
	typedef std::map<const v2<int>, IDSet> GridMap;


	struct Object {
		Object(const v2<int>& pos, const v2<int>& size) : pos(pos), size(size) {}
		v2<int> pos, size;
	};

	void collide(std::set<int> &objects, const GridMap &grid, const v2<int> &grid_size, const v2<int>& area_pos, const v2<int>& area_size) const;
	void removeFromGrid(GridMap &grid, const v2<int> &grid_size, const int id, const Object &o);
	void update(GridMap &grid, const v2<int> &grid_size, const int id, const v2<int> &pos, const v2<int> &size);

	v2<int> _grid_size; 
	v2<int> _grid4_size;
	
	GridMap _grid, _grid4;
	
	typedef std::map<const int, Object> Index;
	Index _index;
};

#endif

