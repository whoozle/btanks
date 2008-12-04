#ifndef QUAD_TREE_H__
#define QUAD_TREE_H__

/* Battle Tanks Game
 * Copyright (C) 2006-2008 Battle Tanks team
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

#include <set>
#include <assert.h>

template<typename T, typename V>
struct quad_point {
	T x, y;
	V value;
	
	quad_point() : x(0), y(0), value(0) {}
	quad_point(const V& v) : x(0), y(0), value(v) {}
	quad_point(const T& x, const T& y, const V& v) : x(x), y(y), value(v) {}
	
	inline bool operator<(const quad_point &other) const {
		if (x != other.x)
			return x < other.x;
		return y < other.y;
	}
};

template<typename T, typename V, int capacity> 
struct quad_node {
	enum { unsplit_capacity = capacity * 2 / 3 };
	typedef quad_point<T, V> point_type;
	typedef quad_node<T, V, capacity> node_type;
	typedef std::multiset<point_type> points_type;
	
	int pos_x, pos_y, size_x, size_y;
	node_type * child[4];
	points_type points;
	
	size_t children_count;

	inline quad_node(int pos_x, int pos_y, int size_x, int size_y): 
		pos_x(pos_x), pos_y(pos_y), size_x(size_x), size_y(size_y), points(), children_count(0) {
			for(int i = 0; i < 4; ++i)
				child[i] = NULL;
		}
		
	inline void set_size(const T&w, const T&h) {
		clear();
		//printf("set_size(%d, %d)\n", w, h);
		size_x = w; size_y = h;
	}
		
	inline int index(const point_type & point) const {
		return index(point.x, point.y);
	}

	inline int index(int x, int y) const {
		return (y * 2 < size_y)? 
			((x * 2 < size_x)? 0: 1): 
			((x * 2 < size_x)? 2: 3);
	}
	
	inline bool empty() const {
		return points.empty() && child[0] == NULL;
	}
	
	void unsplit(points_type &parent) {
		//implement me
		if (child[0] != NULL) {
			assert(points.empty());
			for(int i = 0; i < 4; ++i) {
				child[i]->unsplit(parent);
				delete child[i];
				child[i] = NULL;
			}
		} else {
			parent.insert(points.begin(), points.end());
		}
	}
	
	void split() {
		assert(child[0] == NULL);
		int hx = (size_x - 1) / 2 + 1;
		int hy = (size_y - 1) / 2 + 1;
		//printf("splitting node position %d,%d size %d,%d\n", pos_x, pos_y, size_x, size_y);
		child[0] = new node_type(0, 0, hx, hy);
		child[1] = new node_type(hx, 0, hx, hy);
		child[2] = new node_type(0, hy, hx, hy);
		child[3] = new node_type(hx, hy, hx, hy);
		for(typename points_type::const_iterator i = points.begin(); i != points.end(); ++i) {
			point_type point = *i;
			int idx = index(point);
			node_type * dst_node = child[idx];
			//printf("putting point into quad %d\n", idx);
			point.x -= dst_node->pos_x;
			point.y -= dst_node->pos_y;
			dst_node->insert(point);
		}
		points.clear();
	}

	size_t insert(const point_type & point) {
		if (point.x < 0 || point.x >= size_x || point.y < 0 || point.y >= size_y)
			return false;
		
		size_t n = 0; //points that shares one (x, y)
		{
			typename points_type::iterator b = points.lower_bound(point);
			typename points_type::iterator e = points.upper_bound(point);
			for(typename points_type::iterator i = b; i != e; ++i) {
				++n;
			}
		}
		if (n >= capacity) 
			split();
		
		bool inc;
		if (child[0] != NULL) {
			assert(points.empty());
			node_type * dst_node = child[index(point)];
			point_type p = point;
			p.x -= dst_node->pos_x;
			p.y -= dst_node->pos_y;
			inc = dst_node->insert(p);
		} else {
			typename points_type::iterator i = points.find(point);
			inc = (i != points.end())?1: 0;
			points.insert(point);
		} 
		children_count += inc;
		return inc;
	}

	void search(std::set<V> & result, int x0, int y0, int x1, int y1) const {
		if (child[0] != NULL) {
			assert(points.empty());
			int i0 = index(x0, y0);
			int i1 = index(x1, y1);
			int x, y, dx, dy;
			switch(i1 - i0) {
			case 3: 
				x = y = 0; dx = dy = 1;
				break;

			case 2: 
				x = dx = i0;
				y = 0;
				dy = 1;
				break;

			case 1: 
				y = dy = i1 / 2;
				x = 0;
				dx = 1;
				break;
			
			case 0: 
				y = dy = i0 / 2;
				x = dx = i0 % 2;
				break;

			default: 
				assert(0);
				x = y = 0; dx = dy = -1;
			}
			
			//printf("search: quadrants %d-%d\n", i0, i1);
			for(; y <= dy; ++y) 
				for(; x <= dx; ++x) {
					node_type * dst_node = child[y * 2 + x];
					//printf("search: node %d,%d %d,%d\n", dst_node->pos_x, dst_node->pos_y, dst_node->size_x, dst_node->size_y);
					child[i0]->search(result, x0 - dst_node->pos_x, y0 - dst_node->pos_y, 
						x1 - dst_node->pos_x, y1 - dst_node->pos_y);
				}
		} else {
			//printf("%d,%d: searching in %u points(%d,%d %d,%d)\n", pos_x, pos_y, (unsigned)points.size(), x0, y0, x1, y1);
			for(typename points_type::const_iterator i = points.begin(); i != points.end(); ++i) {
				const point_type &point = *i;
				bool got = false;
				if (point.x >= x0 && point.x < x1 && point.y >= y0 && point.y < y1)	{
					result.insert(point.value);
					got = true;
				}
				//printf("point %d, %d %c\n", point.x, point.y, got? '*': ' ');
			}
		}
	}
	
	size_t erase(const point_type & point) {
		if (point.x < 0 || point.x >= size_x || point.y < 0 || point.y >= size_y)
			return false;
		
		size_t dec;
		if (child[0] != NULL) {
			node_type * dst_node = child[index(point)];
			point_type p = point;
			p.x -= dst_node->pos_x;
			p.y -= dst_node->pos_y;
			dec = dst_node->erase(p);
		} else {
			typename points_type::iterator b = points.lower_bound(point);
			typename points_type::iterator e = points.upper_bound(point);
			dec = 0;
			for(typename points_type::iterator i = b; i != e; ) {
				if (i->value == point.value) {
					points.erase(i++);
					++dec;
				} else 
					++i;
			}
			
		}
		children_count -= dec;
			
		//if (children_count < unsplit_capacity) 
		//	unsplit(points);
		return dec;
	}
	
	void clear() {
		pos_x = 0;
		pos_y = 0;
		size_x = 0;
		size_y = 0;
		for(int i = 0; i < 4; ++i) {
			delete child[i];
			child[i] = NULL;
		}
		children_count = 0;
	}
	
	~quad_node() {
		for(int i = 0; i < 4; ++i) 
			delete child[i];
	}
};

template<typename T, typename V, int capacity>
class quad_tree {
public: 
	typedef quad_point<T, V> point_type;
	typedef quad_node<T, V, capacity> node_type;
	
	inline quad_tree() : root(0, 0, 0, 0) {}
	inline void set_size(const T &w, const T &h) {
		clear();
		root.set_size(w, h);
	}
	
	inline void clear() {
		root.clear();
	}
	
	inline void search(std::set<V> & result, int x0, int y0, int x1, int y1) const {
		if (x0 >= x1 || y0 >= y1)
			return;
		root.search(result, x0, y0, x1, y1);
	}
	
	inline bool insert(const point_type & point) {
		return root.insert(point);
	}

	bool erase(const point_type & point) {
		return root.erase(point);
	}
	
protected: 
	node_type root;
};

#endif

