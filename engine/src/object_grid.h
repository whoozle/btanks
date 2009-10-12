#ifndef BTANKS_OBJECT_GRID_H__
#define BTANKS_OBJECT_GRID_H__

/* Battle Tanks Game
 * Copyright (C) 2006-2009 Battle Tanks team
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
		_size = size;
		tree.set_size(size.x, size.y);
	}

	void clear() {
		tree.clear();
		_index.clear();
		_wrap = false;
		_size.clear();
	}

	void update(T id, const v2<int> &pos, const v2<int> &size) {
		rect_type rect(pos.x, pos.y, pos.x + size.x, pos.y + size.y, id);
		validate(rect);
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
	
	void validate(rect_type & rect) {
		if (_wrap) {
			int w = rect.x1 - rect.x0, h = rect.y1 - rect.y0;
			rect.x0 = wrap(rect.x0, _size.x);
			rect.y0 = wrap(rect.y0, _size.y);
			rect.x1 = rect.x0 + w;
			rect.y1 = rect.y0 + h;
		} else {
			if (rect.x0 < 0)
				rect.x0 = 0;
			if (rect.y0 < 0)
				rect.y0 = 0;
			if (rect.x0 > _size.x)
				rect.x0 = _size.x;
			if (rect.y0 > _size.y)
				rect.y0 = _size.y;

			//copy-paste ninja was here
			if (rect.x1 < 0)
				rect.x1 = 0;
			if (rect.y1 < 0)
				rect.y1 = 0;
			if (rect.x1 > _size.x)
				rect.x1 = _size.x;
			if (rect.y1 > _size.y)
				rect.y1 = _size.y;
		}
	}

	typedef std::map<T, rect_type> Index;
	Index _index;
	v2<int> _size;
	bool _wrap;
};

#endif

