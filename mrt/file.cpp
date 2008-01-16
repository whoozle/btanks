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
#include "file.h"
#include "ioexception.h"
#include "chunk.h"

#include <sys/types.h>
#include <sys/stat.h>

#ifndef _WINDOWS
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

void File::write(const Chunk &ch) const {
	if (fwrite(ch.getPtr(), 1, ch.getSize(), _f) != ch.getSize())
		throw_io(("fwrite"));
}

const bool File::readLine(std::string &str, const size_t bufsize) const {
	if (_f == NULL)
		throw_ex(("readLine on closed file"));
	mrt::Chunk buf;
	buf.setSize(bufsize);
	
	if (fgets((char *)buf.getPtr(), buf.getSize(), _f) == NULL)
		return false;
	str.assign((const char *)buf.getPtr());
	return true;
}

const size_t File::read(void *buf, const size_t size) const {
	size_t r = fread(buf, 1, size, _f);
	if (r == (size_t)-1) 
		throw_io(("read(%p, %u)", buf, (unsigned)size));
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
