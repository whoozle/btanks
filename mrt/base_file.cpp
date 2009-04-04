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

#include "base_file.h"
#include "mrt/chunk.h"
#include "ioexception.h"

using namespace mrt;

#ifdef _WINDOWS
#	ifndef int32_t
#		define int32_t __int32
#	endif
#	ifndef int16_t
#		define int16_t __int16
#	endif
#endif


void BaseFile::readLE16(int &x) const {
	unsigned t; 
	readLE16(t);
	x = (int16_t)t;
}

void BaseFile::readLE32(int &x) const {
	unsigned t; 
	readLE32(t);
	x = (int32_t)t;
}

void BaseFile::readLE16(unsigned int &x) const {
	unsigned char buf[2];
	size_t r = read(buf, 2);
	if (r == (size_t)-1)
		throw_io(("readLE16 failed"));
	if (r != 2)
		throw_ex(("unexpected EOF (read %u of 2 bytes)", (unsigned) r));
	x = buf[0] + (buf[1] << 8);
}

void BaseFile::readLE32(unsigned int &x) const {
	unsigned char buf[4];
	size_t r = read(buf, 4);
	if (r == (size_t)-1)
		throw_io(("readLE16 failed"));
	if (r != 4)
		throw_ex(("unexpected EOF (read %u of 4 bytes)", (unsigned) r));
	x = buf[0] | (buf[1] << 8) | (buf[2] << 16) | (buf[3] << 24);
}

BaseFile::~BaseFile() {
	close();
}

void BaseFile::close() {}

void BaseFile::read_all(std::string &str) const {
	mrt::Chunk data;
	read_all(data);
	str.assign((const char *)data.get_ptr(), data.get_size());
}

void BaseFile::read_all(Chunk &ch) const {
	ch.free();
	
	seek(0, SEEK_SET);
	
#define BUF_SIZE 524288
	long r, size = 0;
	do {
		ch.set_size(size + BUF_SIZE);
		
		unsigned char * ptr = (unsigned char *) ch.get_ptr();
		ptr += size;
		
		r = read(ptr, BUF_SIZE);
		size += r; 
	} while (r == BUF_SIZE);
	ch.set_size(size);
}

void BaseFile::write_all(const Chunk &ch) const {
	seek(0, SEEK_SET);
	write(ch);
}

void BaseFile::write_all(const std::string &str) const {
	mrt::Chunk data;
	data.set_data(str.c_str(), str.size());
	write_all(data);
}

bool BaseFile::readline(std::string &str, const size_t bufsize) const {
	//FIXME FIXME FIXME!!
	//very stupid and sloooow implementation. consider it as a stub. 
	str.clear();
	char c;
	do {
		size_t r = read(&c, 1);
		if (r <= 0)
			return !str.empty();
		str += c;
		if (c == '\n')
			return true;
	} while(true);
}
