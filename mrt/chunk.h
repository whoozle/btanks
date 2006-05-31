#ifndef __CHUNK_H__
#define __CHUNK_H__

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
