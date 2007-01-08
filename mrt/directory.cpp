#include "directory.h"
#include "ioexception.h"
#include <errno.h>

using namespace mrt;

Directory::Directory() : _handle(0) {}
Directory::~Directory() {
	close();
}

const bool Directory::opened() const {
	return _handle != 0;
}

#ifndef WIN32


void Directory::open(const std::string &path) {
	if (path.empty())
		throw_ex(("Directory::open called with empty path"));
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
#include <io.h>

void Directory::open(const std::string &path) {
	struct _finddata_t filedata;
	if ((_handle = _findfirst((path + "/*").c_str(), &filedata)) == -1) {
		_handle = 0;
		throw_io(("findfirst('%s')", path.c_str()));
	}
   	_first_value = filedata.name;
}

const std::string Directory::read() const {
	if (!_first_value.empty()) {
		std::string r = _first_value;
		_first_value.clear();
		return r;
	}
	struct _finddata_t filedata;
	if (_findnext(_handle, &filedata) == 0) {
		return filedata.name;
	}
	return std::string();
}

void Directory::close() {
	if (_handle == 0)
		return;

	_findclose(_handle);
	_handle = 0;
}

#endif
