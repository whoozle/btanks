#ifndef __CHUNK_H__
#define __CHUNK_H__

/* M-runtime for c++
 * Copyright (C) 2005-2008 Vladimir Menshakov
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.

 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
*/

#include <sys/types.h>
#include <string>
#include "export_mrt.h"

namespace mrt {

class MRTAPI Chunk {
public:
	inline Chunk(): ptr(NULL), size(0) {}
	inline Chunk(const Chunk& c) : ptr(NULL), size(0) { *this = c; }
	inline Chunk(const int size): ptr(NULL), size(0) { set_size(size); }
	inline ~Chunk() { free(); }
	inline void *get_ptr() const { return ptr; }
	inline const size_t get_size() const { return size; } 
	inline bool empty() const { return ptr == NULL; }

	//use unlink only if you know what you're doing ;)
	inline void unlink() { ptr = NULL; size = 0; }

	const Chunk& operator=(const Chunk& c);

	void free();
	
	void set_size(size_t s);
	void set_data(const void *p, const size_t s);
	void set_data(void *p, const size_t s, const bool own = false);
	void fill(const int b);
	
	void append(const Chunk &other);
	void move(Chunk &other);
	void append(const void *data, const size_t size);
	void *reserve(const int more);

	const std::string dump() const;

protected:
	void *ptr;
	size_t size;
};

}

#endif
