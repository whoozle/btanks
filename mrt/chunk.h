#ifndef __CHUNK_H__
#define __CHUNK_H__
/* M-Runtime for c++
 * Copyright (C) 2005-2007 Vladimir Menshakov
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

#include <assert.h>
#include <sys/types.h>
#include <string>
#include "export_mrt.h"

namespace mrt {

class MRTAPI Chunk {
public:
	inline Chunk(): ptr(NULL), size(0) {}
	inline Chunk(const Chunk& c) : ptr(NULL), size(0) { *this = c; }
	inline Chunk(const int size): ptr(NULL), size(0) { setSize(size); }
	inline ~Chunk() { free(); }
	inline void *getPtr() const { return ptr; }
	inline const size_t getSize() const { return size; } 

	//use unlink only if you know what you're doing ;)
	inline void unlink() { ptr = 0; size = 0; }

	const Chunk& operator=(const Chunk& c);

	void free();
	
	void setSize(size_t s);
	void setData(const void *p, const size_t s);
	void setData(void *p, const size_t s, const bool own = false);
	void fill(const int b);
	
	void append(const Chunk &other);
	void *reserve(const int more);

	const std::string dump() const;

protected:
	void *ptr;
	size_t size;
};

}

#endif
