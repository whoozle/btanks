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

template<typename T, int C = 8> 
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
		rect_type rect(pos.x, pos.y, pos.x + size.x, pos.y + size.y, id);
		typename Index::iterator i = _index.find(id);
		if (i != _index.end()) {
			rect_type &old = i->second;
			if (old == rect) 
				return;

			tree.erase(old);
			tree.insert(rect);

			old = rect;
		} else {
			_index.insert(typename Index::value_type(id, rect));
			tree.insert(rect);
		}
	}

	void remove(T id) {
		typename Index::iterator i = _index.find(id);
		if (i != _index.end()) {
			rect_type &old = i->second;
			tree.erase(old);
			
			_index.erase(i);
		}
	}

	void collide(std::set<T> &objects, const v2<int>& area_pos, const v2<int>& area_size) const {
		tree.search(objects, rect_type(area_pos.x, area_pos.y, area_pos.x + area_size.x, area_pos.y + area_size.y, NULL));
	}
	
private:
	static inline int wrap(int x, int y) {
		x %= y;
		return x < 0? x + y: x;
	}

	typedef quad_tree<int, T, C> tree_type; 
	typedef typename tree_type::rect_type rect_type;
	tree_type tree;

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
	

	typedef MRT_HASH_MAP <T, rect_type, object_hash > Index;
	Index _index;
	bool _wrap;
};

#endif

