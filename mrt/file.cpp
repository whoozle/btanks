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
#include "file.h"
#include "ioexception.h"
#include "chunk.h"

#include <sys/types.h>
#include <sys/stat.h>

#ifndef WIN32
#include <unistd.h>
#endif

using namespace mrt;

File::File():_f(NULL) {}

void File::open(const std::string &fname, const std::string &mode) {
	_f = fopen(fname.c_str(), mode.c_str());
	if (_f == NULL) 
		throw_io(("fopen(\"%s\", \"%s\")", fname.c_str(), mode.c_str()))
}

const bool File::opened() const {
	return _f != NULL;
}

const off_t File::getSize() const {
	struct stat s;
	int fno = fileno(_f);
	if (fstat(fno, &s) != 0)
		throw_io(("fstat"));
	return s.st_size;
}

void File::readAll(Chunk &ch) const {
	ch.free();
	
	fseek(_f, 0, SEEK_SET);
	
#define BUF_SIZE 16384	
	long r, size = 0;
	do {
		ch.setSize(size + BUF_SIZE);
		unsigned char * ptr = (unsigned char *) ch.getPtr();
		ptr += size;
		
		r = fread(ptr, 1, BUF_SIZE, _f);
		if (r == -1) 
			throw_io(("fread"));
		size += r; 
	} while (r == BUF_SIZE);
	ch.setSize(size);
}

void File::writeAll(const Chunk &ch) const {
	fseek(_f, 0, SEEK_SET);
	write(ch);
}

void File::writeAll(const std::string &str) const {
	fseek(_f, 0, SEEK_SET);
	if (fwrite(str.c_str(), 1, str.size(), _f) != str.size())
		throw_io(("fwrite"));
}

void File::write(const Chunk &ch) const {
	if (fwrite(ch.getPtr(), 1, ch.getSize(), _f) != ch.getSize())
		throw_io(("fwrite"));
}

const bool File::readLine(std::string &str) const {
	char buf[1024];
	
	if (_f == NULL)
		throw_ex(("readLine on closed file"));
	
	if (fgets(buf, sizeof(buf), _f) == NULL)
		return false;
	str = buf;
	return true;
}

const size_t File::read(void *buf, const size_t size) const {
	size_t r = fread(buf, 1, size, _f);
	if (r == (size_t)-1) 
		throw_io(("read(%p, %u)", buf, size));
	return r;
}


const bool File::eof() const {
	int r = feof(_f);
	if (r == -1)
		throw_io(("feof"));
	return r != 0;	
}


void File::close() {
	if (_f != NULL) {
		fclose(_f);
		_f = NULL;
	}
}

FILE * File::unlink() {
	FILE * r = _f;
	_f = NULL;
	return r;
}

int File::seek(long offset, int whence) const {
	if (_f == NULL)
		throw_ex(("seek(%ld, %d) on uninitialized file", offset, whence));
		
	int r = fseek(_f, offset, whence);
	if (r < 0)
		throw_io(("seek(%ld, %d)", offset, whence));
	return r;
}

long File::tell() const {
	if (_f == NULL)
		throw_ex(("tell() on uninitialized file"));
	return ftell(_f);
}
