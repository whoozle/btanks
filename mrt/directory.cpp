#include "directory.h"
#include "ioexception.h"
#include <errno.h>

using namespace mrt;

Directory::Directory() : _handle(0) {}
Directory::~Directory() {
	close();
}

#ifndef WIN32


void Directory::open(const std::string &path) {
	_handle = opendir(path.c_str());
	if (_handle == NULL)
		throw_io(("opendir('%s')", path.c_str()));
}

const std::string Directory::read() const {
	if (_handle == NULL)
		throw_ex(("Directory::read called on uninitialized object"));
	struct dirent *dir = readdir(_handle);
	if (dir == NULL) {
		if (errno) 
			throw_io(("readdir"));
		return std::string();
	}
	return dir->d_name;
}

void Directory::close() {
	if (_handle != NULL) {
		closedir(_handle);
		_handle = NULL;
	}
}

#else 
//
#endif
