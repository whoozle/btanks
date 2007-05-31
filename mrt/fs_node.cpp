#include "fs_node.h"

#include <sys/types.h>
#include <sys/stat.h>

#ifndef WIN32
#	include <unistd.h>
#endif

#include "mrt/exception.h"

using namespace mrt;

const bool FSNode::exists(const std::string &fname) {
	struct stat buf;
	return stat(fname.c_str(), &buf) == 0;
}

const std::string FSNode::getDir(const std::string &fname) {
	std::string::size_type p = fname.rfind('/');
	if (p == fname.npos)
		throw_ex(("getDir('%s') failed", fname.c_str()));
	
	if (p == 0)
		return fname;
	
	return fname.substr(0, p - 1);
}


const std::string FSNode::relativePath(const std::string &from_dir, const std::string &to_dir) {
	std::vector<std::string> f_path, t_path;
	mrt::split(f_path, from_dir, "/");
	mrt::split(t_path, to_dir, "/");
	throw_ex(("implement me"));
	return std::string();
}
