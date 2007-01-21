#include "fs_node.h"

#include <sys/types.h>
#include <sys/stat.h>

#ifndef WIN32
#	include <unistd.h>
#endif

using namespace mrt;

const bool FSNode::exists(const std::string &fname) {
	struct stat buf;
	return stat(fname.c_str(), &buf) == 0;
}
