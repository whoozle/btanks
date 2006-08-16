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

const bool BaseAI::getPath(Matrix<int> &result, const v3<float> dpos) {
	Matrix<int> imp, path;
	World->getImpassabilityMatrix(imp);
	
	v3<int> src, dst;
	
	{
		v3<float> p;
		convertToAbsolute(p, dpos);
		p /= IMap::pathfinding_step;
		dst = p.convert<int>();
		
		getPosition(p);
		p /= IMap::pathfinding_step;
		src = p.convert<int>();
	}
	
	int w = imp.getWidth(), h = imp.getHeight();

	path.setSize(h, w, -1);
	path.useDefault(-1);
	//path.set(src.y, src.x, 0);
	
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
	
	if (n == -1)
		return false;
	result.setSize(h, w, -1);
	int x = dst.x, y = dst.y;
	int i = 0;
	
	while ( x != src.x || y != src.y) {
		result.set(y, x, i++);
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
	result.set(y, x, i++);
	
	//LOG_DEBUG(("getPath: length: %d, \n%s", len, result.dump().c_str()));
	return true;
}
