#ifndef __MRT_DIRECTORY_H__
#define __MRT_DIRECTORY_H__

#ifndef _WINDOWS
#	include <sys/types.h>
#	include <dirent.h>
#endif
#include "base_directory.h"
#include "fs_node.h"

namespace mrt { 

class MRTAPI Directory : public BaseDirectory, public FSNode {
public: 

	Directory();
	virtual void create(const std::string &path, const bool recurse = false);
	virtual void open(const std::string &path);
	virtual const bool opened() const;
	virtual const std::string read() const;
	virtual void close();
	virtual ~Directory();
	
	static const std::string getHome();
	static const std::string getAppDir(const std::string &name, const std::string &shortname);
	
private: 

#ifdef _WINDOWS
	typedef long dir_t;
	mutable std::string _first_value;
#else
	typedef DIR * dir_t;
#endif

	dir_t _handle;
};
}

#endif

