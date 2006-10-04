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

const off_t File::getSize() const {
	struct stat s;
	int fno = fileno(_f);
	fstat(fno, &s);
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
	if (fwrite(ch.getPtr(), 1, ch.getSize(), _f) != ch.getSize())
		throw_io(("fwrite"));
}

void File::writeAll(const std::string &str) const {
	fseek(_f, 0, SEEK_SET);
	if (fwrite(str.c_str(), 1, str.size(), _f) != str.size())
		throw_io(("fwrite"));
}

const std::string File::readLine() const {
	char buf[1024];
	
	if (_f == NULL)
		throw_ex(("readLine on closed file"));
	
	if (fgets(buf, sizeof(buf), _f) == NULL)
		throw_io(("fgets"));
	return buf;
}

const size_t File::read(void *buf, const size_t size) const {
	return fread(buf, 1, size, _f);
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
