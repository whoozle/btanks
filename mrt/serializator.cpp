/* M-runtime for c++
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

#ifndef WIN32
#	include <arpa/inet.h>
#else
#	include <winsock2.h>
#	ifndef snprintf
#		define snprintf _snprintf
#	endif
#endif

#include "serializator.h"
#include "chunk.h"
#include <assert.h>
#include <limits.h>
#include "exception.h"

#define IEEE_754_SERIALIZATION

using namespace mrt;

#define INTEGER 0x80
#define SIGNED_INTEGER 0x40

#define FLOAT 1
#define STRING 2

#define ASSERT_POS(size) if (_pos + (size) > _data->getSize()) throw_ex(("buffer overrun %d + %d > %d", _pos, (size), _data->getSize()))

//ugly hackish trick, upcast const pointer to non-const variant.

Serializator::Serializator() : _data(new mrt::Chunk), _pos(0), _owns_data(true) {}
Serializator::Serializator(const mrt::Chunk *chunk) : _data((mrt::Chunk *)chunk), _pos(0), _owns_data(false) {}

Serializator::~Serializator() {
	if (_owns_data) {
		delete _data;
		_data = NULL;
	}
}

const bool Serializator::end() const {
	return _pos >= _data->getSize();
}

void Serializator::add(const int n) {
	//LOG_DEBUG(("added int %d", n));
	unsigned char type = INTEGER;
	unsigned char buf[sizeof(unsigned long)];
	
	int x = n, len;
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

void Serializator::add(const unsigned int n) {
	add((int)n);
}

void Serializator::get(unsigned int &n) const {
	int *p = (int *)&n;
	get(*p);
}

void Serializator::add(const bool b) {
	//LOG_DEBUG(("added bool %c", b?'t':'f'));
	add((int) (b?'t':'f'));
}

void Serializator::add(const std::string &str) {
	//LOG_DEBUG(("added string %s", str.c_str()));
	int size = str.size();
	add(size);
	
	unsigned char *ptr = (unsigned char *) _data->reserve(size) + _pos;
	memcpy(ptr, str.c_str(), size);
	_pos += size;
}

void Serializator::add(const Chunk &c) {
	int size = c.getSize();
	add(size);
	if (size == 0)
		return;

	unsigned char *ptr = (unsigned char *) _data->reserve(size) + _pos;
	memcpy(ptr, c.getPtr(), size);
	_pos += size;
}

void Serializator::add(const void *raw, const int size) {
	add(size);
	if (size == 0)
		return;

	unsigned char *ptr = (unsigned char *) _data->reserve(size) + _pos;
	memcpy(ptr, raw, size);
	_pos += size;
}

void Serializator::add(const float f) {
#ifdef IEEE_754_SERIALIZATION
	add(&f, sizeof(f));
#else
	//LOG_DEBUG(("added float %f", f));
	char buf[256];
	unsigned int len = snprintf(buf, sizeof(buf) -1, "%g", f);
	add(std::string(buf, len));
#endif
}

void Serializator::get(int &n)  const {
	unsigned char * ptr = (unsigned char *) _data->getPtr();

	ASSERT_POS(1);
	unsigned char type = *(ptr + _pos++);
	if (! (type & INTEGER))
		throw_ex(("got %02x('%c'), instead of integer type-len byte", type, type>=0x20?type:'.'));
	unsigned char len = type & 0x3f;
	ASSERT_POS(len);
	
	if (len == 0) {
		n = 0; 
	} else if(len == sizeof(unsigned char)) {
		n = *(ptr + _pos++);
	} else if (len == sizeof(unsigned short)) {
		n = ntohs(*((unsigned short *)(ptr + _pos)));
		_pos += sizeof(unsigned short);
	} else if (len == sizeof(unsigned long)) {
		n = ntohl(*((unsigned long *)(ptr + _pos)));
		_pos += sizeof(unsigned long);
	}
	if (type & SIGNED_INTEGER) 
		n = -n;
}

void Serializator::get(bool &b) const {
	int x;
	get(x);
	if (x != 't' && x != 'f')
		throw_ex(("invalid boolean value '%02x'", x));
	b = x == 't';
}

void Serializator::get(float &f) const {
#ifdef IEEE_754_SERIALIZATION
	int size;
	get(size);
	if (size != sizeof(f))
		throw_ex(("failed to deserialize IEEE 754 float"));	
	get((void *)&f, size);
#else
	std::string str;
	get(str);
	if (sscanf(str.c_str(), "%f", &f) != 1)
		throw_ex(("failed to cast '%s' to float", str.c_str()));
#endif
}

void Serializator::get(std::string &str)  const {
	unsigned int size;
	get(size);

	ASSERT_POS(size);
	const char * ptr = (const char *) _data->getPtr() + _pos;
	str = std::string(ptr, size);
	_pos += size;
}

void Serializator::get(void *raw, const int size) const {
	ASSERT_POS(size);
	if (size == 0) 
		return;
	
	const char * ptr = (const char *) _data->getPtr() + _pos;
	memcpy(raw, ptr, size);
	_pos += size;
}


void Serializator::get(Chunk &c)  const {
	int size;
	get(size);

	ASSERT_POS(size);
	c.setSize(size);
	
	if (size == 0) 
		return;
	
	const char * ptr = (const char *) _data->getPtr() + _pos;
	memcpy(c.getPtr(), ptr, size);
	_pos += size;
}


const Chunk & Serializator::getData() const {
	return *_data;
}
