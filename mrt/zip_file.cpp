#include "zip_file.h"
#include "ioexception.h"
#include "logger.h"

using namespace mrt;

ZipFile::ZipFile(FILE *file, const unsigned method, const unsigned flags, const unsigned offset, const unsigned csize, const unsigned usize) : 
file(file), method(method), flags(flags), offset(offset), csize(csize), usize(usize), voffset(0) {
	if (method != 0)
		throw_ex(("compression method %u unsupported", method));
}

void ZipFile::open(const std::string &fname, const std::string &mode) {
	throw_ex(("unimplemented!"));
}

const bool ZipFile::opened() const {
	return file != NULL;
}

int ZipFile::seek(long offset, int whence) const {
	switch(whence) {
	case SEEK_SET: 
	case SEEK_CUR:
	case SEEK_END: 
		break;
	}
}

long ZipFile::tell() const {
	return voffset;
}

void ZipFile::write(const mrt::Chunk &ch) const {
	throw_ex(("unimplemented!"));
}

const off_t ZipFile::getSize() const {
	return usize;
}

const size_t ZipFile::read(void *buf, const size_t size) const {
	size_t rsize = size;
	if (rsize > usize - voffset)
		rsize = usize - voffset;
	size_t r = fread(buf, 1, rsize, file);
	if (r == (size_t)-1) 
		throw_io(("read(%p, %u)", buf, (unsigned)size));
	return size;
}

void ZipFile::close() {
	if (file == NULL)
		return;
	fclose(file);
	file = NULL;
}
	
const bool ZipFile::eof() const {
	return voffset < usize;
}

const bool ZipFile::readLine(std::string &str, const size_t bufsize) const {

}

ZipFile::~ZipFile() {
	close();
}
