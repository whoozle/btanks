#include "fs_node.h"

#include <sys/types.h>
#include <sys/stat.h>

#ifndef WIN32
#	include <unistd.h>
#endif

#include "mrt/exception.h"
#include <deque>

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

static void pack_path(std::deque<std::string> &result, const std::vector<std::string> &path, const size_t start) {
	result.clear();
	for(size_t i = start; i < path.size(); ++i) {
		const std::string &e = path[i];
		if (e == ".")
			continue;
		if (e == ".." && !result.empty())
			result.pop_back();
		result.push_back(e);
	}
}

const std::string FSNode::relativePath(const std::string &from_dir, const std::string &to_dir) {
	std::vector<std::string> f_path, t_path;
	mrt::split(f_path, from_dir, "/");
	mrt::split(t_path, to_dir, "/");

	size_t base = 0;
	for(; base < f_path.size() && base < t_path.size(); ++base) {
		if (f_path[base] != t_path[base])
			break;
	}

	std::deque<std::string> f, t;
	pack_path(f, f_path, base);
	pack_path(t, t_path, base);	
	
	std::vector<std::string> result;
	for(size_t i = 0; i < f.size(); ++i) 
		result.push_back("..");
	for(size_t i = 0; i < t.size(); ++i) 
		result.push_back(t[i]);
	
	std::string r_str;
	join(r_str, result, "/");
	return r_str;
}
