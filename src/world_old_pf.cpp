//BIG PATHFINDING PART

typedef v3<int> vertex;
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
/*
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
	
	return r * 100 / 41;
}
*/
	
const bool IWorld::old_findPath(const Object *obj, const v3<float>& position, Way & way, const Object *dst_obj) const {
	//finding shortest path.
	v3<float> tposition = obj->_position + position;
	
	Matrix<int> imp, path;
	World->getImpassabilityMatrix(imp, obj, dst_obj);
	//LOG_DEBUG(("imp\n%s", imp.dump().c_str()));
	
	v3<int> src = obj->_position.convert<int>() / IMap::pathfinding_step;
	v3<int> dst = tposition.convert<int>() / IMap::pathfinding_step;
	
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
		
		if (imp.get(v.y + 1, v.x) != -1)
			push(path, buf, vertex(v.x, v.y + 1, n));
		if (imp.get(v.y - 1, v.x) != -1)
			push(path, buf, vertex(v.x, v.y - 1, n));
		if (imp.get(v.y, v.x + 1) != -1)
			push(path, buf, vertex(v.x + 1, v.y, n));
		if (imp.get(v.y, v.x - 1) != -1)
			push(path, buf, vertex(v.x - 1, v.y, n));
/*
		//disabled diagonals for now
		if (check(imp, v, 1, 1) != -1)
			push(path, buf, vertex(v.x + 1, v.y + 1, n));
		if (check(imp, v, 1, -1) != -1)
			push(path, buf, vertex(v.x + 1, v.y - 1, n));
		if (check(imp, v, -1, 1) != -1)
			push(path, buf, vertex(v.x - 1, v.y + 1, n));
		if (check(imp, v, -1, -1) != -1)
			push(path, buf, vertex(v.x - 1, v.y - 1, n));
*/
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
		/*
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
		*/
		assert(t != -1);
		
		x = x2; y = y2; n = t;
	}
	//way.push_front(WayPoint(x, y, 0));
	//LOG_DEBUG(("imp\n%s", imp.dump().c_str()));
	
	
	for(Way::iterator i = way.begin(); i != way.end(); ++i) {
		(*i) *= IMap::pathfinding_step;
	}
	
	//LOG_DEBUG(("getPath: length: %d, \n%s", len, result.dump().c_str()));
	return true;
}
