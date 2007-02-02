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
	void setSize(const int w, const int h, const int step);

	void clear();
	void update(const int id, const v2<int> &pos, const v2<int> &size);
	void remove(const int id);

	void collide(std::set<int> &objects, const int id);
	void collide(std::set<int> &objects, const v2<int>& area_pos, const v2<int>& area_size);
	
private:
	struct Object {
		Object(const v2<int>& pos, const v2<int>& size) : pos(pos), size(size) {}
		v2<int> pos, size;
	};

	void removeFromGrid(const int id, const Object &o);

	v2<int> _grid_size; 
	int _step;
	typedef std::set<int> IDSet;
	typedef std::map<const v2<int>, IDSet> GridMap;
	GridMap _grid;
	
	typedef std::map<const int, Object> Index;
	Index _index;
};

#endif

