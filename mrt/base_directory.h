#ifndef __MRT_BASE_DIRECTORY_H__
#define __MRT_BASE_DIRECTORY_H__

#include <string>
#include "fs_node.h"
#include "export_mrt.h"

namespace mrt { 

class MRTAPI BaseDirectory {
public: 
	virtual void open(const std::string &path) = 0;
	virtual const bool opened() const = 0;
	virtual const std::string read() const = 0;
	virtual void close() = 0;
	virtual void create(const std::string &path) = 0;
	virtual ~BaseDirectory() = 0;
};
}

#endif

