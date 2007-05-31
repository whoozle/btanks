#ifndef __MRT_FS_NODE_H__
#define __MRT_FS_NODE_H__

#include <string>
#include "export_mrt.h"

namespace mrt {

class MRTAPI FSNode {
public:
	static const bool exists(const std::string &fname);
	static const std::string getDir(const std::string &fname);
	static const std::string relativePath(const std::string &from_dir, const std::string &to_dir);
};

}

#endif
