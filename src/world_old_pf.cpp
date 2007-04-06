
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

#define USE_ASTAR

#ifdef USE_ASTAR
struct Point {
	Point() {}
	v2<int> id, parent;
	int g, h;

};

struct PD {
	int f;
	v2<int> id;
	PD(const int f, const v2<int> &id) : f(f), id(id) {}
	const bool operator<(const PD &other) const {
		return f > (other.f);
	}
};

typedef std::set<v2<int> > CloseList;
typedef std::priority_queue<PD> OpenList;
typedef std::map<const v2<int>, Point> PointMap;

static inline const int h(const v2<int>& src, const v2<int>& dst) {
	return 500 * (math::abs(src.x - dst.x) + math::abs<int>(src.y - dst.y));
}


const bool IWorld::old_findPath(const Object *obj, const v2<float>& position, Way & way, const Object *dst_obj) const {
	//finding shortest path.
	v2<float> tposition = obj->_position + position;
	
	Matrix<int> imp, path;
	World->getImpassabilityMatrix(imp, obj, dst_obj);
	//LOG_DEBUG(("imp\n%s", imp.dump().c_str()));
	v2<int> tile_size = Map->getPathTileSize();
	//LOG_DEBUG(("pathfinding tile size reported: %d %d", tile_size.x, tile_size.y));

	v2<int> src = obj->_position.convert<int>() / tile_size;
	v2<int> dst = tposition.convert<int>() / tile_size;
	
	if (src == dst) {
		way.push_back(dst);
		return true;
	}
	
	int max_w = imp.getWidth(), max_h = imp.getHeight();

	way.clear();
	OpenList _open_list;
	PointMap _points;
	CloseList _close_list;
	
	Point p;
	p.id = src;
	p.g = 0;
	p.h = h(p.id, dst);

	_open_list.push(PD(p.g + p.h, p.id));
	_points[p.id] = p;

	const int dirs = obj->getDirectionsNumber();
	if (dirs < 4 || dirs > 8)
		throw_ex(("pathfinding cannot handle directions number: %d", dirs));

	while(!_open_list.empty()) {
		PD pd = _open_list.top();
		PointMap::const_iterator pi = _points.find(pd.id);
		assert(pi != _points.end());
		
		const Point &current = pi->second;
		_open_list.pop();
		
		assert(current.id.x >= 0 && current.id.x < max_w && current.id.y >= 0 && current.id.y < max_h);
		if (_close_list.find(current.id) != _close_list.end())
			continue;
		_close_list.insert(current.id);
		
		
		for(int i = 0; i < dirs; ++i) {
			v2<float> d;
			v2<int> id;
			d.fromDirection(i, dirs);
			id.x = (int)math::sign(d.x);
			id.y = (int)math::sign(d.y);
			id += current.id;
						
			if (_close_list.find(id) != _close_list.end())
				continue;
			
			if (id.x >= max_w || id.x < 0 || id.y < 0 || id.y >= max_h)
				continue;

			int im = imp.get(id.y, id.x);
			if (im < 0) {
				_close_list.insert(id);
				continue;
			}
			
			Point p;
			p.id = id;
			p.parent = current.id;
			p.g = current.g + ((d.x != 0 && d.y != 0)?141:100) + im;
			p.h = h(id, dst);

			PointMap::iterator pi = _points.find(id);
			
			if (pi != _points.end()) {
				if (pi->second.g > p.g) {
					pi->second = p;
				}
			} else 
				_points.insert(PointMap::value_type(id, p));
			
			
			if (p.h < 100) {
				dst = p.id;
				goto found;
			}

			_open_list.push(PD(p.g + p.h, p.id));
		}			
		
	}
	
	way.clear();
	return false;
	
found: 
	for(v2<int> id = dst; id != src; ) {
		Point &p = _points[id];
		way.push_front(p.id);
		//LOG_DEBUG(("%dx%d -> %dx%d", p.id % _pitch, p.id / _pitch, way.front().x, way.front().y));
		assert(id != p.parent);
		id = p.parent;
	}


	//way.push_front(WayPoint(x, y, 0));
	//LOG_DEBUG(("imp\n%s", imp.dump().c_str()));
	
	
	for(Way::iterator i = way.begin(); i != way.end(); ++i) {
		(*i) *= tile_size;
	}
	
	//LOG_DEBUG(("getPath: length: %d, \n%s", len, result.dump().c_str()));
	return true;
}


#else


//BIG PATHFINDING PART

#undef DISABLE_PF_DIAGONALS


typedef v2<int> vertex;
typedef std::deque<vertex> vertex_queue;

static inline void push(Matrix<int> &path, vertex_queue &buf, const vertex &vertex) {
	int w = path.get(vertex.y, vertex.x);
	if (w != -1 && w <= vertex.z) 
		return;
	path.set(vertex.y, vertex.x, vertex.z);
	buf.push_back(vertex);
}

static inline const bool pop(vertex_queue &buf, vertex &vertex) {
	if (buf.empty())
		return false;
	vertex = buf.front();
	buf.pop_front();
	return true;
}


inline static const int check(const Matrix<int> &imp, const vertex &v, const int dx, const int dy) {
	int w;
	if ((w = imp.get(v.y, v.x)) == -1)
		return -1;
	int r = w;
	
	if ((w = imp.get(v.y, v.x + dx)) == -1)
		return -1;
	if (w > r) r = w;

	if ((w = imp.get(v.y + dy, v.x)) == -1)
		return -1;
	if (w > r) r = w;

	if ((w = imp.get(v.y + dy, v.x + dx)) == -1)
		return -1;
	if (w > r) r = w;
	
	return r * 1414 / 1000;
}

	
const bool IWorld::old_findPath(const Object *obj, const v2<float>& position, Way & way, const Object *dst_obj) const {
	//finding shortest path.
	v2<float> tposition = obj->_position + position;
	
	Matrix<int> imp, path;
	World->getImpassabilityMatrix(imp, obj, dst_obj);
	//LOG_DEBUG(("imp\n%s", imp.dump().c_str()));
	v2<int> tile_size = Map->getPathTileSize();
	//LOG_DEBUG(("pathfinding tile size reported: %d %d", tile_size.x, tile_size.y));

	v2<int> src = obj->_position.convert<int>() / tile_size;
	v2<int> dst = tposition.convert<int>() / tile_size;
	
	int w = imp.getWidth(), h = imp.getHeight();

	path.setSize(h, w, -1);
	path.useDefault(-1);
	
	vertex_queue buf;
	imp.set(src.y, src.x, 0);
	push(path, buf, vertex(src.x, src.y, 0));
	
	vertex v;
	while(pop(buf, v)) {
		int n = path.get(v.y, v.x);
		assert(n != -1);
		
		int w = imp.get(v.y, v.x);
		//LOG_DEBUG(("get(%d, %d) = %d, %d", v.y, v.x, w, n));
		assert(w != -1);
		if (w == -1)
			continue;

		n += w + 1;
		
		if (imp.get(v.y + 1, v.x) >= 0)
			push(path, buf, vertex(v.x, v.y + 1, n));
		if (imp.get(v.y - 1, v.x) >= 0)
			push(path, buf, vertex(v.x, v.y - 1, n));
		if (imp.get(v.y, v.x + 1) >= 0)
			push(path, buf, vertex(v.x + 1, v.y, n));
		if (imp.get(v.y, v.x - 1) >= 0)
			push(path, buf, vertex(v.x - 1, v.y, n));
#ifndef DISABLE_PF_DIAGONALS
		//disabled diagonals for now
		if (check(imp, v, 1, 1) >= 0)
			push(path, buf, vertex(v.x + 1, v.y + 1, n));
		if (check(imp, v, 1, -1) >= 0)
			push(path, buf, vertex(v.x + 1, v.y - 1, n));
		if (check(imp, v, -1, 1) >= 0)
			push(path, buf, vertex(v.x - 1, v.y + 1, n));
		if (check(imp, v, -1, -1) >= 0)
			push(path, buf, vertex(v.x - 1, v.y - 1, n));
#endif
	}
	
	int len, n = path.get(dst.y, dst.x);
	len = n;
	
	if (n == -1) {
		/*
		imp.set(dst.y, dst.x, -99);
		imp.set(src.y, src.x, imp.get(src.y, src.x) - 100);
		*/
		LOG_DEBUG(("path not found from %d:%d -> %d:%d", src.y, src.x, dst.y, dst.x));
		//LOG_DEBUG(("imp\n%s", imp.dump().c_str()));
		//LOG_DEBUG(("path\n%s", path.dump().c_str()));
		return false;
	}

	way.clear();
	int x = dst.x, y = dst.y;
	int vi = -10;
	
	while ( x != src.x || y != src.y) {
		assert(imp.get(y, x) != -1);
		imp.set(y, x, vi--);
		way.push_front(WayPoint(x, y, 0));
		int t = n;
		int x2 = x, y2 = y;

		int w = path.get(y + 1, x);
		if (w != -1 && w < t) {
			x2 = x; y2 = y + 1; t = w;
		}
		w = path.get(y - 1, x);
		if (w != -1 && w < t) {
			x2 = x; y2 = y - 1;	t = w;
		}
		w = path.get(y, x + 1);
		if (w != -1 && w < t) {
			x2 = x + 1; y2 = y; t = w;
		}
		w = path.get(y, x - 1);
		if (w != -1 && w < t) {
			x2 = x - 1; y2 = y; t = w;
		}
		//diagonals 
#ifndef DISABLE_PF_DIAGONALS
		w = path.get(y + 1, x + 1);
		if (w != -1 && w < t) {
			y2 = y + 1; x2 = x + 1; t = w;
		}
		w = path.get(y + 1, x - 1);
		if (w != -1 && w < t) {
			y2 = y + 1; x2 = x - 1; t = w;
		}
		w = path.get(y - 1, x + 1);
		if (w != -1 && w < t) {
			y2 = y - 1; x2 = x + 1; t = w;
		}
		w = path.get(y - 1, x - 1);
		if (w != -1 && w < t) {
			y2 = y - 1; x2 = x - 1; t = w;
		}
#endif

		assert(t != -1);
		
		x = x2; y = y2; n = t;
	}
	//way.push_front(WayPoint(x, y, 0));
	//LOG_DEBUG(("imp\n%s", imp.dump().c_str()));
	
	
	for(Way::iterator i = way.begin(); i != way.end(); ++i) {
		(*i) *= tile_size;
	}
	
	//LOG_DEBUG(("getPath: length: %d, \n%s", len, result.dump().c_str()));
	return true;
}
#endif
