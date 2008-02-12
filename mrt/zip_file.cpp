#include "zip_file.h"
#include "exception.h"

using namespace mrt;

ZipFile::ZipFile(FILE *file, const unsigned method, const unsigned flags, const unsigned offset, const unsigned csize, const unsigned usize) : 
file(file), method(method), flags(flags), offset(offset), csize(csize), usize(usize), voffset(0) {
}

void ZipFile::open(const std::string &fname, const std::string &mode) {
	throw_ex(("unimplemented!"));
}

const bool ZipFile::opened() const {
	return true;
}

int ZipFile::seek(long offset, int whence) const {
	
}

long ZipFile::tell() const {

}

void ZipFile::write(const mrt::Chunk &ch) const {

}

const off_t ZipFile::getSize() const {

}

const size_t ZipFile::read(void *buf, const size_t size) const {

}

void ZipFile::close() {

}
	
const bool ZipFile::eof() const {

}

const bool ZipFile::readLine(std::string &str, const size_t bufsize) const {

}
