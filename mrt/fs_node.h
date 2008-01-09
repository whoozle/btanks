#ifndef __MRT_FS_NODE_H__
#define __MRT_FS_NODE_H__

#include <string>
#include "export_mrt.h"

namespace mrt {

class MRTAPI FSNode {
public:
	virtual ~FSNode() {}
	virtual bool exists(const std::string &fname) const;
	static const std::string getDir(const std::string &fname);
	static const std::string getParentDir(const std::string &fname);
	static const std::string getFilename(const std::string &fname, const bool return_ext = true);
	static const std::string relativePath(const std::string &from_dir, const std::string &to_dir);
};

}

#endif
