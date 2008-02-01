#include "mrt_zzip_dir.h"
#include "mrt_zzip_file.h"
#include "mrt/ioexception.h"

using namespace zzip;

Directory::Directory() : _dir(NULL) {}

void Directory::open(const std::string &path) {
	close();
	//zzip_error_t code;
	sdlx::AutoMutex m(big_lock);
	_dir = zzip_opendir_ext_io(path.c_str(), ZZIP_THREADED | ZZIP_ONLYZIP, NULL, NULL);
	if (_dir == NULL) 
		throw_io(("could not open archive file %s", path.c_str()));
}

const bool Directory::opened() const {
	return _dir != NULL;
}

const std::string Directory::read() const {
	ZZIP_DIRENT dirent;
	sdlx::AutoMutex m(big_lock);
	if (zzip_dir_read(_dir, &dirent) != 1)
		return std::string();
	
	return dirent.d_name;
}

void Directory::close() {
	if (_dir == NULL)
		return;

	sdlx::AutoMutex m(big_lock);
	zzip_dir_close(_dir);
	_dir = NULL; //leak resources to avoid race with global dtors.
}

void Directory::create(const std::string &path, const bool recurse) {
	throw_ex(("implement me"));
}

Directory::~Directory() {
	close();
}

File * Directory::open_file(const std::string &name) const {
	int mode = O_RDONLY;
#ifdef _WINDOWS 
	mode |= O_BINARY;
#endif
	sdlx::AutoMutex m(big_lock);
	ZZIP_FILE *f = zzip_file_open(_dir, name.c_str(), mode);
	if (f == NULL)
		throw_io(("zzip_file_open"));
	return new File(f, big_lock);
}
