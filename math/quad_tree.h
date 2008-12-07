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
#include <stdio.h>

template<typename T, typename V>
struct quad_rect {
	T x0, y0, x1, y1;
	V value;
	
	quad_rect() : x0(0), y0(0), x1(0), y1(0), value(0) {}
	quad_rect(const V& v) : x0(0), y0(0), x1(0), y1(0), value(v) {}
	quad_rect(const T& x0, const T& y0, const T& x1, const T& y1, const V& v) : x0(x0), y0(y0), x1(x1), y1(y1), value(v) {}
	
	inline bool operator<(const quad_rect &other) const {
		if (x0 != other.x0)
			return x0 < other.x0;
		if (y0 != other.y0)
			return x0 < other.x0;
		if (x1 != other.x1)
			return x1 < other.x1;
		return y1 < other.y1;
	}

	inline bool operator==(const quad_rect &other) const {
		return y0 == other.y0 && y1 == other.y1 && x0 == other.x0 && x1 == other.x1;
	}

	inline bool valid() const {
		return x0 < x1 && y0 < y1;
	}
	
	bool contains(const quad_rect & other) const {
		return x0 <= other.x0 && x1 >= other.x1 && y0 <= other.y0 && y1 >= other.y1;
	}

	inline bool intersects(const quad_rect & other) const {
		return !( x0 >= other.x1 || x1 <= other.x0 ||
				y0 >= other.y1 || y1 <= other.y0 );
	}
	
	inline void clear() {
		x0 = 0;
		x1 = 0;
		y0 = 0;
		y1 = 0;
	}
};

template<typename T, typename V, int capacity> 
struct quad_node {
	enum { unsplit_capacity = capacity * 2 / 3 };
	typedef quad_rect<T, V> rect_type;
	typedef quad_node<T, V, capacity> node_type;
	typedef std::list<rect_type> rects_type;
	
	
	rect_type area;
	rects_type rects;
	quad_node * child[4];
	size_t children_count;
	
	quad_node(const rect_type &area) : area(area), rects(), children_count(0) {
		child[0] = NULL;
		child[1] = NULL;
		child[2] = NULL;
		child[3] = NULL;
	}
	~quad_node() {
		for(int i = 0; i < 4; ++i) {
			delete child[i];
			child[i] = NULL;
		}
	}
	
	void set_size(const T & w, const T & h) {
		clear();
		area.x1 = w;
		area.y1 = h;
	}
	
	void clear() {
		area.clear();
		rects.clear();
		for(int i = 0; i < 4; ++i) {
			delete child[i];
			child[i] = NULL;
		}
		children_count = NULL;
	}
	
	void merge(std::set<V> &result) const {
		if (child[0] != NULL)
			for(int j = 0; j < 4; ++j) {
				child[j]->merge(result);
			}
			
		for(typename rects_type::const_iterator i = rects.begin(); i != rects.end(); ++i) {
			const rect_type &r = *i;
			result.insert(r.value);
		}
	}
	
	void search(std::set<V> & result, const rect_type &s_area) const {
		if (!area.intersects(s_area))
			return;

		if (child[0] != NULL)
		for(int j = 0; j < 4; ++j) {
			if (s_area.contains(child[j]->area)) {
				//move all area to result :)
				child[j]->merge(result);
			} else {
				child[j]->search(result, s_area);
			}
		}

		for(typename rects_type::const_iterator i = rects.begin(); i != rects.end(); ++i) {
			const rect_type & rect = *i;
			if (rect.intersects(s_area))
				result.insert(rect.value);
		}
	}
	
	void split() {
		assert(child[0] == NULL);
		
		int w = area.x1 - area.x0, h = area.y1 - area.y0;
		if (w < 2 || h < 2)
			return;

		//printf("splitting node %d,%d,%d,%d (%u)\n", area.x0, area.y0, area.x1, area.y1, (unsigned)children_count);
		
		int hx = (w - 1) / 2 + 1;
		int hy = (h - 1) / 2 + 1;
		child[0] = new quad_node(rect_type(area.x0, 		area.y0, 		area.x0 + hx, 	area.y0 + hy, NULL));
		child[1] = new quad_node(rect_type(area.x0 + hx, 	area.y0, 		area.x1, 		area.y0 + hy, NULL));
		child[2] = new quad_node(rect_type(area.x0, 		area.y0 + hy, 	area.x0 + hx, 	area.y1, NULL));
		child[3] = new quad_node(rect_type(area.x0 + hx, 	area.y0 + hy, 	area.x1, 		area.y1, NULL));
	}
	
	bool insert(const rect_type & object) {
		if (!area.contains(object))
			return false;
		
		if (child[0] == NULL) 
			split();

		if (child[0] != NULL)
		for(int i = 0; i < 4; ++i) {
			if (child[i]->insert(object)) {
				++children_count;
				return true;
			}
		}
	
		//printf("insert %d,%d,%d,%d into %d,%d,%d,%d [%d]\n", object.x0, object.y0, object.x1, object.y1, area.x0, area.y0, area.x1, area.y1, children_count);
		rects.push_back(object);
		++children_count;
		return true;
	}

	bool erase(const rect_type & object) {
		if (!area.contains(object))
			return false;

		if (child[0] != NULL)
		for(int i = 0; i < 4; ++i) 
			if (child[i]->erase(object)) {
				--children_count;
				return true;
			}
		
		//printf("erase %d,%d,%d,%d into %d,%d,%d,%d [%d]\n", object.x0, object.y0, object.x1, object.y1, area.x0, area.y0, area.x1, area.y1, children_count);
		for(typename rects_type::iterator i = rects.begin(); i != rects.end(); ++i) {
			const rect_type &r = *i;
			if (r == object && r.value == object.value) {
				i = rects.erase(i);
				--children_count;
				return true;
			}
		}
		return false;
	}
	
};

template<typename T, typename V, int capacity>
class quad_tree {
public: 
	typedef quad_rect<T, V> rect_type;
	typedef quad_node<T, V, capacity> node_type;
	
	inline quad_tree() : root(rect_type()) {}
	inline void set_size(const T &w, const T &h) {
		clear();
		root.set_size(w, h);
	}
	
	inline void clear() {
		root.clear();
	}
	
	inline void search(std::set<V> & result, const rect_type &area) const {
		if (!area.valid())
			return;

		if (root.area.contains(area)) {
			root.search(result, area);
		} else {
			rect_type child[4]; //splitting rectangle
			int n = split(child, area);
			for(int i = 0; i < n; ++i) {
				root.search(result, child[i]);
			}
		}
	}
	
	inline void insert(const rect_type & object) {
		if (!object.valid())
			return;
		if (root.area.contains(object)) {
			root.insert(object);
		} else {
			rect_type child[4]; //splitting rectangle
			int n = split(child, object);
			for(int i = 0; i < n; ++i) {
				root.insert(child[i]);
			}
		}
	}

	inline void erase(const rect_type & object) {
		if (!object.valid())
			return;
		if (root.area.contains(object)) {
			root.erase(object);
		} else {
			rect_type child[4]; //splitting rectangle
			int n = split(child, object);
			for(int i = 0; i < n; ++i) {
				root.erase(child[i]);
			}
		}
	}
	
protected: 
	inline int split(rect_type * child, const rect_type &rect) const {
		bool has_x = rect.x1 > root.area.x1;
		bool has_y = rect.y1 > root.area.y1;
		if (has_x && has_y) {
			child[0] = rect_type(rect.x0, rect.y0, root.area.x1, root.area.y1, rect.value);
			child[1] = rect_type(0, rect.y0, rect.x1 - root.area.x1, root.area.y1, rect.value);

			child[2] = rect_type(rect.x0, 0, root.area.x1, rect.y1 - root.area.y1, rect.value);
			child[3] = rect_type(0, 0, rect.x1 - root.area.x1, rect.y1 - root.area.y1, rect.value); 
			return 4;
		} else if (has_x) {
			child[0] = rect_type(rect.x0, rect.y0, root.area.x1, rect.y1, rect.value);
			child[1] = rect_type(0, rect.y0, rect.x1 - root.area.x1, rect.y1, rect.value);
			return 2;
		} else if (has_y) {
			child[0] = rect_type(rect.x0, rect.y0, rect.x1, root.area.y1, rect.value);
			child[1] = rect_type(rect.x0, 0, rect.x1, rect.y1 - root.area.y1, rect.value);
			return 2;
		} else {
			child[0] = rect;
			return 1;
		}
	}

	node_type root;
};

#endif

