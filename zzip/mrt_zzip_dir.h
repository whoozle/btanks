#ifndef __ZZIP_MRT_DIRECTORY_H__
#define __ZZIP_MRT_DIRECTORY_H__

#include <string>
#include "export_zzip.h"
#include "zzip/zzip.h"
#include "sdlx/mutex.h"

namespace zzip { 

class File;
class ZZIPAPI Directory {
public: 
	sdlx::Mutex big_lock; /// zzlib are not thread safe at all. :(((

	Directory();
	virtual void open(const std::string &path);
	virtual const bool opened() const;
	virtual const std::string read() const;
	virtual void close();
	virtual void create(const std::string &path);
	virtual ~Directory();
	File * open_file(const std::string &name) const;
private: 
	ZZIP_DIR * _dir;
};
}

#endif

