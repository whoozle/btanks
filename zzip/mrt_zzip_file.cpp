#include "mrt_zzip_file.h"
#include "mrt/ioexception.h"

using namespace zzip;

zzip::File::File() : _f(NULL), _endof(true) {}

const bool File::readLine(std::string &str, const size_t bufsize) const {
	throw_ex(("unimplemented"));
}

void File::open(const std::string &fname, const std::string &mode) {
	close();
	_f = zzip_fopen(fname.c_str(), mode.c_str());
	if (_f == NULL)
		throw_io(("zzip_fopen(%s, %s)", fname.c_str(), mode.c_str()));
	_endof = false;
}

const bool File::opened() const {
	return _f != NULL;
}

int File::seek(long offset, int whence) const {
	return zzip_seek(_f, offset, whence);
}

long File::tell() const {
	return zzip_tell(_f);
}

void File::write(const mrt::Chunk &ch) const {
	throw_ex(("unimplemented"));
}

const off_t File::getSize() const {
	ZZIP_STAT stat;
	if (zzip_file_stat(_f, &stat) == -1)
		throw_io(("zzip_file_stat"));
	return stat.st_size;
}

const size_t File::read(void *buf, const size_t size) const {
	int r = zzip_read(_f, buf, size);
	if (r < 0) 
		throw_io(("zzip_read"));
	if ((size_t)r != size)
		_endof = true;
	return r;
} 
void File::close() {
	if (_f == NULL)
		return;
	
	zzip_close(_f);
	_f = NULL;
	_endof = true;
}
	
const bool File::eof() const{
	return _endof;
}

File::~File() {
	close();
}
