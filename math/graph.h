#ifndef __BTANKS_GRAPH_H__
#define __BTANKS_GRAPH_H__

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
#include <limits>
#include "mrt/exception.h"

template <class T> class Graph {
public:
	void insert(int a, int b, const T w) {
		if (a > b) {
			int c = b; b = a; a = c;
		} else if (a == b) {
			throw_ex(("inserting edge (%d, %d) in graph (same vertexes)", a, b));
		}
		_edges[EdgeMap::key_type(a, b)] = w;
	}
	const T get(int a, int b) const {
		if (a > b) {
			int c = b; b = a; a = c;
		} else if (a == b) {
			throw_ex(("getting edge (%d, %d) from graph (same vertexes)", a, b));
		}
		typename EdgeMap::const_iterator i = _edges.find(EdgeMap::key_type(a, b));
		if (i == _edges.end())
			return std::numeric_limits<T>::infinity();
		return i->second;
	}
private:
	typedef std::map<const std::pair<int, int> , T> EdgeMap;
	EdgeMap _edges;
};

#endif
