#include "base_ai.h"
#include "world.h"
#include "tmx/map.h"
#include <deque>

BaseAI::BaseAI(const bool stateless) : Player(stateless) {}
BaseAI::BaseAI(const std::string &animation, const bool stateless) : Player(animation, stateless) {}

typedef v3<int> vertex;
typedef std::deque<vertex> vertex_queue;

static void push(Matrix<int> &path, vertex_queue &buf, const vertex &vertex) {
	int w = path.get(vertex.y, vertex.x);
	if (w != -1 && w <= vertex.z) 
		return;
	path.set(vertex.y, vertex.x, vertex.z);
	buf.push_back(vertex);
}

static const bool pop(vertex_queue &buf, vertex &vertex) {
	if (buf.empty())
		return false;
	vertex = buf.front();
	buf.pop_front();
	return true;
}

static void clear(Matrix<int> &m, const v3<int>& p1, const v3<int> &size) {
	v3<int> p2 = p1 + size - 1;
	for(int y = p1.y/IMap::pathfinding_step; y <= p2.y/IMap::pathfinding_step; ++y) 
		for(int x = p1.x/IMap::pathfinding_step; x <= p2.x/IMap::pathfinding_step; ++x) {
		m.set(y, x, 0);
	}
}

const bool BaseAI::getPath(Way &result, const v3<float> &dpos) {
	Matrix<int> imp, path;
	World->getImpassabilityMatrix(imp);
	//LOG_DEBUG(("imp\n%s", imp.dump().c_str()));
	
	v3<int> src, dst;
	
	{
		v3<float> p;
		convertToAbsolute(p, dpos);
		clear(imp, p.convert<int>(), size);
		p /= IMap::pathfinding_step;
		dst = p.convert<int>();
		
		getPosition(p);
		clear(imp, p.convert<int>(), size);
		p /= IMap::pathfinding_step;
		src = p.convert<int>();
	}
	
	int w = imp.getWidth(), h = imp.getHeight();

	path.setSize(h, w, -1);
	path.useDefault(-1);
	
	vertex_queue buf;
	push(path, buf, vertex(src.x, src.y, 0));
	
	vertex v;
	while(pop(buf, v)) {
		int n = path.get(v.y, v.x);
		assert(n != -1);
		int w = imp.get(v.y, v.x);
		//LOG_DEBUG(("get(%d, %d) = %d, %d", v.y, v.x, w, n));
		assert(w != -1);
		//if (w == -1)
		//	continue;

		n += w + 1;
		
		if (imp.get(v.y + 1, v.x) != -1)
			push(path, buf, vertex(v.x, v.y + 1, n));
		if (imp.get(v.y - 1, v.x) != -1)
			push(path, buf, vertex(v.x, v.y - 1, n));
		if (imp.get(v.y, v.x + 1) != -1)
			push(path, buf, vertex(v.x + 1, v.y, n));
		if (imp.get(v.y, v.x - 1) != -1)
			push(path, buf, vertex(v.x - 1, v.y, n));
	}
	
	int len, n = path.get(dst.y, dst.x);
	len = n;
	
	if (n == -1) {
		imp.set(dst.y, dst.x, -99);
		LOG_DEBUG(("imp\n%s", imp.dump().c_str()));
		return false;
	}

	result.clear();
	int x = dst.x, y = dst.y;
	int i = -10;
	
	while ( x != src.x || y != src.y) {
		assert(imp.get(y, x) != -1);
		imp.set(y, x, i--);
		result.push_front(WayPoint(x, y, 0));
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
		x = x2; y = y2; n = t;
	}
	//result.push_front(WayPoint(x, y, 0));
	LOG_DEBUG(("imp\n%s", imp.dump().c_str()));
	
	
	for(Way::iterator i = result.begin(); i != result.end(); ++i) {
		(*i) *= IMap::pathfinding_step;
	}
	
	//LOG_DEBUG(("getPath: length: %d, \n%s", len, result.dump().c_str()));
	return true;
}
