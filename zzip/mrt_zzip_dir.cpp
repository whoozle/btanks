#include "mrt_zzip_dir.h"
#include "mrt_zzip_file.h"
#include "mrt/exception.h"

using namespace zzip;

Directory::Directory() : _dir(NULL) {}

void Directory::open(const std::string &path) {
	close();
	zzip_error_t code;
	_dir = zzip_dir_open(path.c_str(), &code);
	if (_dir == NULL) 
		throw_ex(("could not open archive file %s with code: %d", path.c_str(), (int) code));
}

const bool Directory::opened() const {
	return _dir != NULL;
}

const std::string Directory::read() const {
	ZZIP_DIRENT dirent;
	if (zzip_dir_read(_dir, &dirent) != 1)
		return std::string();
	
	return dirent.d_name;
}

void Directory::close() {
	if (_dir == NULL)
		return;
	zzip_dir_close(_dir);
	_dir = NULL;
}

void Directory::create(const std::string &path) {

}

Directory::~Directory() {
	close();
}
