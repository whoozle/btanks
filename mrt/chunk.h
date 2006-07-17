#ifndef __CHUNK_H__
#define __CHUNK_H__
/* M-Runtime for c++
 * Copyright (C) 2005-2006 Vladimir Menshakov
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

namespace mrt {

class Chunk {
public:
	Chunk() : ptr(0), size(0) {};

	void free();
	//use unlink only if you know what you doing ;)
	void unlink() { ptr = 0; size = 0; }
	const Chunk& operator=(const Chunk& c);

	void setSize(size_t s);
	void setData(const void *ptr, const size_t size);
	void setData(void *ptr, const size_t size, const bool takeOwnership = false);
	
	void append(const Chunk &other);

	inline void *getPtr() const { return ptr; }
	inline const size_t getSize() const { return size; }
	
	~Chunk();
	
	inline unsigned char& operator[](const unsigned int p) {
		assert(ptr);
		assert(p<size);
	    return *(((unsigned char *)ptr) + p);
	}

	const unsigned char& operator[](const unsigned int p) const {
		assert(ptr);
		assert(p<size);
	    return *(((unsigned char *)ptr) + p);
	}
	
	const std::string dump() const;

protected:
	void *ptr;
	size_t size;
private:
	Chunk(const Chunk&) {}
};

}

#endif
