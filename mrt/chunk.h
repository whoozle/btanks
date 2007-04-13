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
#include "export.h"

namespace mrt {

class MRTAPI Chunk {
public:
	Chunk();
	Chunk(const Chunk&);
	Chunk(const int size);
	const Chunk& operator=(const Chunk& c);

	void free();
	~Chunk();
	
	//use unlink only if you know what you're doing ;)
	void unlink() { ptr = 0; size = 0; }

	void setSize(size_t s);
	void setData(const void *p, const size_t s);
	void setData(void *p, const size_t s, const bool own = false);
	void fill(const int b);
	
	void append(const Chunk &other);
	void *reserve(const int more);

	inline void *getPtr() const { return ptr; }
	inline const size_t getSize() const { return size; }
	
	
	const std::string dump() const;

protected:
	void *ptr;
	size_t size;
};

}

#endif
