#ifndef __ZZIP_MRT_DIRECTORY_H__
#define __ZZIP_MRT_DIRECTORY_H__

#include <string>
#include "export_zzip.h"
#include "zzip/zzip.h"

namespace zzip { 

class ZZIPAPI Directory {
public: 
	Directory();
	virtual void open(const std::string &path);
	virtual const bool opened() const;
	virtual const std::string read() const;
	virtual void close();
	virtual void create(const std::string &path);
	virtual ~Directory();
private: 
	ZZIP_DIR * _dir;
};
}

#endif

