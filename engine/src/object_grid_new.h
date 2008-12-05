#ifndef BTANKS_OBJECT_GRID_H__
#define BTANKS_OBJECT_GRID_H__

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

#include <map>
#include <string>
#include <set>
#include <algorithm>
#include "math/v2.h"
#include "math/binary.h"
#include "math/quad_tree.h"

template<typename T, int C = 32> 
class Grid {
public: 
	Grid() {}

	void set_size(const v2<int> &size, const int step, const bool wrap) {
		clear();
		_wrap = wrap;
		tree.set_size(size.x, size.y);
	}

	void clear() {
		tree.clear();
		_index.clear();
		_wrap = false;
	}

	void update(T id, const v2<int> &pos, const v2<int> &size) {
		typename Index::iterator i = _index.find(id);
		if (i != _index.end()) {
			v2<int> pos2 = pos + size;
			
			Object &o = i->second;
			if (o.pos == pos && o.pos2 == pos2) 
				return;

			tree.erase(value_type(o.pos.x, o.pos.y, id)); //ul
			tree.insert(value_type(pos.x, pos.y, id));
			
			tree.erase(value_type(o.pos2.x, o.pos2.y, id)); //dr
			tree.insert(value_type(pos2.x, pos2.y, id));

			tree.erase(value_type(o.pos.x, o.pos2.y, id)); //dl
			tree.insert(value_type(pos.x, pos2.y, id));
			
			tree.erase(value_type(o.pos2.x, o.pos.y, id)); //ur
			tree.insert(value_type(pos2.x, pos.y, id));

			o.pos = pos;
			o.pos2 = pos2;
		} else {
			Object o(pos, size);
			_index.insert(typename Index::value_type(id, o));
			
			tree.insert(value_type(o.pos.x, o.pos.y, id));
			tree.insert(value_type(o.pos2.x, o.pos2.y, id));

			tree.insert(value_type(o.pos.x, o.pos2.y, id));
			tree.insert(value_type(o.pos2.x, o.pos.y, id));
		}
	}

	void remove(T id) {
		typename Index::iterator i = _index.find(id);
		if (i != _index.end()) {
			Object &o = i->second;
			tree.erase(value_type(o.pos.x, o.pos.y, id));
			tree.erase(value_type(o.pos2.x, o.pos2.y, id));

			tree.erase(value_type(o.pos.x, o.pos2.y, id));
			tree.erase(value_type(o.pos2.x, o.pos.y, id));
			
			_index.erase(i);
		}
	}

	void collide(std::set<T> &objects, const v2<int>& area_pos, const v2<int>& area_size) const {
		tree.search(objects, area_pos.x, area_pos.y, area_pos.x + area_size.x, area_pos.y + area_size.y);
	}
	
private:
	static inline int wrap(int x, int y) {
		x %= y;
		return x < 0? x + y: x;
	}

	struct Object {
		Object(const v2<int>& pos, const v2<int>& size) : pos(pos), pos2(pos + size) {}
		v2<int> pos, pos2;
	};

	struct object_hash {
		inline size_t operator()(const ::Object *o) const { 
			size_t x = (size_t)o;
			size_t r = 0x1b766561;
			for(int i = 0; i < 4; ++i) {
				size_t c = (x >> (8 * i)) & 0xff;
				r ^= ((r << 5) + c + (r >> 2));
			}
			return r;
		}
	};
	
	typedef MRT_HASH_MAP <T, Object, object_hash > Index;
	
	typedef quad_tree<int, T, C> tree_type; 
	typedef typename tree_type::point_type value_type;
	tree_type tree;
	
	Index _index;
	bool _wrap;
};

#endif

