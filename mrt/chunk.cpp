#include "chunk.h"
#include <stdlib.h>
#include <string.h>
#include "ioexception.h"
#include "fmt.h"

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


using namespace mrt;

void Chunk::fill(const int b) {
	if (ptr == NULL) 
		return;
	memset(ptr, b, size);
}

const Chunk& Chunk::operator=(const Chunk& c) {
	if (this == &c) 
		return *this; // same object

	if (c.ptr == NULL) {
		free();
		return *this;
	}
	assert(c.size > 0);

	void *p = realloc(ptr, c.size);
	if (p == NULL) 
		throw_io(("realloc (%p, %u)", ptr, (unsigned)c.size));
	ptr = p;
	size = c.size;
	memcpy(ptr, c.ptr, c.size);
	return *this;
}

void Chunk::set_size(size_t s) {
	if (s == size)
		return;
	
	if (s == 0) {
		free();
		return;
	}

	void * x = realloc(ptr, s);
	if (x == NULL) 
		throw_io(("realloc (%p, %u)", ptr, (unsigned)s));
	ptr = x;
	size = s;
}

void Chunk::set_data(const void *p, const size_t s) {
	if (p == NULL || s == 0)
		throw_ex(("calling set_data(%p, %u) is invalid", p, (unsigned)s));

	void *x = realloc(ptr, s);

	if (x == NULL) 
		throw_io(("realloc (%p, %d)", ptr, (unsigned)s));
	ptr = x;
	memcpy(ptr, p, s);
	size = s;
}

void Chunk::set_data(void *p, const size_t s, const bool own) {
	if (p == NULL || s == 0) 
		throw_ex(("calling set_data(%p, %u, %s) is invalid", p, (unsigned)s, own?"true":"false"));
	
	if (own) {
		free();
		ptr = p;
		size = s;
	} else {
		void *x = realloc(ptr, s);
		if (x == NULL) 
			throw_io(("realloc(%p, %d)", ptr, (unsigned)s));
		ptr = x;
		size = s;
		memcpy(ptr, p, s);
	}
}

void Chunk::append(const Chunk &other) {
	size_t s1 = size, s2 = other.get_size();
	if (s2 == 0)
		return;
	set_size(s1 + s2);
	memcpy((char *) ptr + s1, other.ptr, s2);
}

void Chunk::append(const void *data, const size_t data_size) {
	if (data_size == 0)
		return;

	size_t s = size;
	set_size(s + data_size);
	memcpy((char *) ptr + s, data, data_size);
}


void* Chunk::reserve(const int more) {
	set_size(size + more);
	return ptr;
}

void Chunk::free() {
	if (ptr != NULL) {
		::free(ptr);
		ptr = NULL;
		size = 0;
	}
}

const std::string Chunk::dump() const {
	if (ptr == NULL)
		return "empty memory chunk";
	assert(ptr != 0);
	
	std::string result = format_string("-[memory dump]-[size: %u]---", (unsigned)size);
	size_t n = (size - 1)/ 16 + 1;
	for(size_t i = 0; i < n; ++i) {
		result += mrt::format_string("\n%06x\t", (unsigned)(i * 16));
		size_t j, m = (size - i * 16);
		if (m > 16) 
			m = 16;
		
		for(j = 0; j < m; ++j) {
			const unsigned char *p = ((unsigned char *)ptr) + i*16 + j;
			result += format_string("%02x ", *p);
			if (j == 7) 
				result += " ";
		}
		for(; j < 16; ++j) {
			if (j == 7) 
				result += " ";
			result += "   ";
		}
		result += "\t\t";

		for(j = 0; j < m; ++j) {
			const unsigned char *p = ((unsigned char *)ptr) + i*16 + j;
			result += format_string("%c", (*p>=32 && *p < 127)? *p: '.');
			if (j == 7) 
				result += " ";
		}
		for(; j < m; ++j) {
			if (j == 7) 
				result += " ";
			result += " ";
		}
	}
	return result;
}

