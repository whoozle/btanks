#ifndef __MRT_DIRECTORY_H__
#define __MRT_DIRECTORY_H__

#ifndef WIN32
#	include <sys/types.h>
#	include <dirent.h>
#endif
#include <string>

namespace mrt { 

class Directory {
public: 

	Directory();
	void open(const std::string &path);
	const std::string read() const;
	void close();
	~Directory();
	
private: 


#ifdef WIN32
	typedef long dir_t;
	mutable std::string _first_value;
#else
	typedef DIR * dir_t;
#endif

	dir_t _handle;
};
}

#endif

