#include "mrt_zzip_file.h"

using namespace zzip;

File::File() {}

const bool File::readLine(std::string &str, const size_t bufsize) const {
	return false;
}

void File::open(const std::string &fname, const std::string &mode) {
}

const bool File::opened() const {
	return false;
}

int File::seek(long offset, int whence) const {
	return -1;
}

long File::tell() const {
	return -1;
}

void File::write(const mrt::Chunk &ch) const {
}

const off_t File::getSize() const {
	return (off_t)-1;
}
const size_t File::read(void *buf, const size_t size) const {
	return (size_t)-1;
} 
void File::close() {
}
	
const bool File::eof() const{
	return true;
}

File::~File() {
	close();
}
