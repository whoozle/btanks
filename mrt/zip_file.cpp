#include "zip_file.h"
#include "ioexception.h"
#include "logger.h"
#include <errno.h>

using namespace mrt;

ZipFile::ZipFile(FILE *file, const unsigned method, const unsigned flags, const unsigned offset, const unsigned csize, const unsigned usize) : 
file(file), method(method), flags(flags), offset(offset), csize(csize), usize(usize), voffset(0) {
	if (method != 0)
		throw_ex(("compression method %u unsupported", method));
	//LOG_DEBUG(("file created with usize: %ld", this->usize));
	if (fseek(file, offset, SEEK_SET) == -1)
		throw_io(("fseek(%u, SEEK_SET)", offset));
}

void ZipFile::open(const std::string &fname, const std::string &mode) {
	throw_ex(("unimplemented!"));
}

bool ZipFile::opened() const {
	return file != NULL;
}

void ZipFile::seek(long off, int whence) const {
	switch(whence) {
	case SEEK_SET: {
		if (off < 0 || off > usize) 
			throw_ex(("seek(%ld, SEEK_SET) jumps out of file (%ld)", off, usize));
		
		long r = fseek(file, offset + off, whence);
		if (r == -1)
			throw_io(("fseek"));
		break;
	}
	case SEEK_CUR: {
		if (off + voffset < 0 || off + voffset >= usize) 
			throw_ex(("seek(%ld, SEEK_CUR) jumps out of file (%ld inside %ld)", off, voffset, usize));

		long r = fseek(file, off, whence);
		if (r == -1)
			throw_io(("fseek"));
		break;
	}
	case SEEK_END: {
		if (off + usize < 0 || off > 0) 
			throw_ex(("seek(%ld, SEEK_END) jumps out of file (size: %ld)", off,  usize));
		
		if (fseek(file, off + usize + offset, SEEK_SET) == -1)
			throw_io(("fseek"));
		break;
	}
	default: 
		throw_ex(("seek: unknown whence value (%d)", whence));
	}
	voffset = ftell(file) - offset;
	if (voffset < 0 || voffset > usize)
		throw_ex(("invalid voffset(%ld) after seek operation", voffset));
}

long ZipFile::tell() const {
	return voffset;
}

void ZipFile::write(const mrt::Chunk &ch) const {
	throw_ex(("unimplemented!"));
}

const off_t ZipFile::get_size() const {
	return usize;
}

const size_t ZipFile::read(void *buf, const size_t size) const {
	size_t rsize = ((long)size > usize - voffset)? usize - voffset: size;
	//LOG_DEBUG(("read(%u), offset: %u + %ld, rsize: %u, usize: %u", (unsigned) size, offset, voffset, (unsigned)rsize, (unsigned)usize));
	size_t r = fread(buf, 1, rsize, file);
	if (r == (size_t)-1) 
		throw_io(("read(%p, %u)", buf, (unsigned)size));
	//LOG_DEBUG(("result = %lu", (unsigned long) r));
	voffset = ftell(file) - offset;
	if (voffset < 0 || voffset > usize)
		throw_ex(("invalid voffset(%ld) after seek operation", voffset));
	return r;
}

void ZipFile::close() {
	if (file == NULL)
		return;
	fclose(file);
	file = NULL;
}
	
bool ZipFile::eof() const {
	return voffset < usize;
}

ZipFile::~ZipFile() {
	close();
}
