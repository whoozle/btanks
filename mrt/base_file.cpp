
/* M-Runtime for c++
 * Copyright (C) 2005-2008 Vladimir Menshakov
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

#include "base_file.h"
#include "mrt/chunk.h"
#include "ioexception.h"

using namespace mrt;

BaseFile::~BaseFile() {
	close();
}

void BaseFile::close() {}

void BaseFile::readAll(std::string &str) const {
	mrt::Chunk data;
	readAll(data);
	str.assign((const char *)data.getPtr(), data.getSize());
}

void BaseFile::readAll(Chunk &ch) const {
	ch.free();
	
	seek(0, SEEK_SET);
	
#define BUF_SIZE 524288
	long r, size = 0;
	do {
		ch.setSize(size + BUF_SIZE);
		
		unsigned char * ptr = (unsigned char *) ch.getPtr();
		ptr += size;
		
		r = read(ptr, BUF_SIZE);
		size += r; 
	} while (r == BUF_SIZE);
	ch.setSize(size);
}

void BaseFile::writeAll(const Chunk &ch) const {
	seek(0, SEEK_SET);
	write(ch);
}

void BaseFile::writeAll(const std::string &str) const {
	mrt::Chunk data;
	data.setData(str.c_str(), str.size());
	writeAll(data);
}

