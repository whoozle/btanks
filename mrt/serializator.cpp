#include "serializator.h"
#include "chunk.h"
#include <assert.h>
#include <limits.h>
#include <arpa/inet.h>
#include "exception.h"

using namespace mrt;

#define INTEGER 0x80
#define SIGNED_INTEGER 0x40

#define FLOAT 1
#define STRING 2

//ugly hackish trick, upcast const pointer to non-const variant.

Serializator::Serializator() : _data(new mrt::Chunk), _pos(0), _owns_data(true) {}
Serializator::Serializator(const mrt::Chunk *chunk) : _data((mrt::Chunk *)chunk), _pos(0), _owns_data(false) {}

Serializator::~Serializator() {
	if (_owns_data) {
		delete _data;
		_data = NULL;
	}
}

void Serializator::add(const size_t n) {

	unsigned char type = INTEGER;
	unsigned char buf[sizeof(unsigned long)];
	
	size_t x = n, len;
	if (x < 0) {
		type |= SIGNED_INTEGER;
		x = -x;
	}
	assert(x >= 0);
	if (x == 0) {
		len = 0;
	} else if (x <= UCHAR_MAX) {
		buf[0] = (unsigned char)x;
		len = sizeof(unsigned char);
	} else if (x <= USHRT_MAX) {
		* (unsigned short *)buf = htons(x);
		len = sizeof(unsigned short);
	} else {
		* (unsigned long *)buf = htonl(x);
		len = sizeof(unsigned long);
	}

	unsigned char *ptr = (unsigned char *) _data->reserve(1 + len) + _pos;
	*ptr++ = type | len;
	memcpy(ptr, buf, len);
	_pos += len + 1;
}

void Serializator::add(const int n) {
	add((size_t)n);
}

void Serializator::add(const std::string &str) {
	size_t size = str.size();
	add(size);
	
	unsigned char *ptr = (unsigned char *) _data->reserve(size) + _pos;
	memcpy(ptr, str.c_str(), size);
	_pos += size;
}

void Serializator::add(const float f) {
	char buf[256];
	snprintf(buf, sizeof(buf) -1, "%f", f);
	buf[sizeof(buf) -1] = 0;
	add(buf);
}

void Serializator::get(size_t &n)  const {
	unsigned char * ptr = (unsigned char *) _data->getPtr();
	unsigned char type = *(ptr + _pos++);
	if (! (type & INTEGER))
		throw_ex(("got %02x, instead of integer type-len byte", type));
	unsigned char len = type & 0x3f;
	if (len == 0) {
		n = 0; 
	} else if(len == sizeof(unsigned char)) {
		n = *(ptr + _pos++);
	} else if (len == sizeof(unsigned short)) {
		n = ntohs(*((unsigned short *)(ptr + _pos)));
		_pos += sizeof(unsigned short);
	} else if (len == sizeof(unsigned long)) {
		n = ntohs(*((unsigned long *)(ptr + _pos)));
		_pos += sizeof(unsigned long);
	}
}

void Serializator::get(int &n) const {
	get((size_t&)n);
}

void Serializator::get(float &f) const {
	std::string str;
	get(str);
	if (sscanf(str.c_str(), "%f", &f) != 1)
		throw_ex(("failed to cast '%s' to float", str.c_str()));
}

void Serializator::get(std::string &str)  const {
	size_t size;
	get(size);

	const char * ptr = (const char *) _data->getPtr() + _pos;
	str = std::string(ptr, size);
	_pos += size;
}


const Chunk & Serializator::getData() const {
	return *_data;
}
