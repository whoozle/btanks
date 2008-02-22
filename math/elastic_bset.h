#ifndef BT_ELASTIC_BSET_H__
#define BT_ELASTIC_BSET_H__

#include <cstddef>
#include <stdlib.h>
#include <stdexcept>
#include <vector>
#include <string.h>
#include <assert.h>

template<typename T, unsigned page_size = 1024> 
class elastic_bset {
public: 
	typedef T  key_type;

private: 
	key_type * data;
	size_t size;

public: 
	elastic_bset() : data(0), size(0) {}
	static inline size_t align(const size_t size) {
		return (1 + (size - 1) / page_size) * page_size;
	}
	~elastic_bset() {
		::free(data);
	}
	
	void resize(size_t new_size) {
		if (new_size == 0)
			return;
		
		new_size = align(new_size);
		
		if (new_size <= size)
			return;
		
		key_type *p = static_cast<key_type *>(::realloc(data, new_size));
		if (p == NULL)
			throw std::runtime_error("realloc failed");
		//printf("realloc(%u)\n", (unsigned)new_size);
		
		data = p;
		size = new_size;

		::memset((char *)data + size, 0, new_size - size);
	}
	
	void insert(const unsigned value) {
		resize((value > 0)? ((value - 1) / sizeof(key_type) / 8 + 1): 1);
		const unsigned bit = value % ( sizeof(key_type) * 8 );
		const unsigned offset = value / sizeof(key_type) / 8;
		assert(offset >= 0 && offset < size);
		data[offset] |= (key_type)1 << bit;
	}
	
	void erase(const unsigned &value) {
		const unsigned bit = value % ( sizeof(key_type) * 8 );
		const unsigned offset = value / sizeof(key_type) / 8;
		if (offset >= size)
			return;
		data[offset] &= ~((key_type)1 << bit);
		//add shrinking from head
	}
	
	const elastic_bset<key_type, page_size>& operator&= (const elastic_bset<key_type, page_size>& other) {
		size_t msize = size < other.size? size: other.size;
		for(size_t i = 0; i < msize; ++i) {
			data[i] &= other.data[i];
		}
	}

	const elastic_bset<key_type, page_size>& operator|= (const elastic_bset<key_type, page_size>& other) {
		size_t msize = size < other.size? size: other.size;
		for(size_t i = 0; i < msize; ++i) {
			data[i] |= other.data[i];
		}
	}

	template<typename K>
	void store(std::vector<K> &values) {
		for(size_t i = 0; i < size; ++i) {
			key_type k = data[i];
			if (k == 0)
				continue;
			for(size_t j = 0; j < sizeof(key_type) * 8; ++j) {
				if (((key_type)1 << j) != 0) 
					values.push_back(i * sizeof(key_type) * 8 + j);
			}
		}
	}
};

#endif

