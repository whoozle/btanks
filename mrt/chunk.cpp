#include "chunk.h"
#include <stdlib.h>
#include "ioexception.h"
#include "fmt.h"

using namespace mrt;

const Chunk& Chunk::operator=(const Chunk& c) {
    free();
    if ((ptr = malloc(c.size)) == NULL) 
		throw_io(("malloc"));
    size = c.size;
    memcpy(ptr, c.ptr, c.size);
    return *this;
}

void Chunk::setSize(size_t s) {
	if (s == 0) {
		if (size == 0) return;
		free();
	} else {
		void * x = realloc(ptr, s);
		if (x == NULL) 
			throw_io(("realloc (%p, %d)", ptr, s));
		ptr = x;
		size = s;
	}
}

void Chunk::setData(const void *p, const size_t s) {
	if (p == NULL || s == 0) {
		free();
	} else {
		void *x = realloc(ptr, s);
		if (x == NULL) 
			throw_io(("realloc (%p, %d)", ptr, s));
		ptr = x;
		memcpy(ptr, p, s);
	}
}

void Chunk::setData(void *p, const size_t s, const bool takeOwnership) {
	if (takeOwnership) {
		ptr = p;
		size = s;
	} else {
		void *x = realloc(ptr, s);
		if (x == NULL) 
			throw_io(("realloc(%p, %d)", ptr, s));
		ptr = x;
		memcpy(ptr, p, s);
	}
}

void Chunk::append(const Chunk &other) {
	size_t s1 = size, s2 = other.getSize();
	if (s2 == 0)
		return;
	setSize(s1 + s2);
	memcpy((char *) ptr + s1, other.ptr, s2);
}

void Chunk::reserve(const int more) {
	setSize(size + more);
}

void Chunk::free() {
	if (ptr != NULL) {
		::free(ptr);
		ptr = NULL;
	}
	size = 0;
}

Chunk::~Chunk() {
	free();
}

const std::string Chunk::dump() const {
	if (size == 0)
		return "empty memory chunk";
	assert(ptr != 0);
	
	std::string result = formatString("-[memory dump]-[size: %d]---", size);
	size_t n = (size - 1)/ 16 + 1;
	for(size_t i = 0; i < n; ++i) {
		result += "\n";
		size_t j, m = (size - i * 16);
		if (m > 16) 
			m = 16;
		
		for(j = 0; j < m; ++j) {
			const unsigned char *p = ((unsigned char *)ptr) + i*16 + j;
			result += formatString("%02x ", *p);
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
			result += formatString("%c", (*p>=32 && *p < 128)? *p: '.');
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
