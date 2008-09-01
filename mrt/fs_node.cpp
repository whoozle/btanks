/* M-runtime for c++
 * Copyright (C) 2005-2008 Vladimir Menshakov
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.

 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
*/

#include "fs_node.h"

#include <sys/types.h>
#include <sys/stat.h>

#ifndef _WINDOWS
#	include <unistd.h>
#endif

#include "mrt/exception.h"
#include <deque>

using namespace mrt;

const std::string FSNode::get_parent_dir(const std::string &fname) {
	std::string::size_type p = fname.rfind('\\'), p2 = fname.rfind('/');
	if (p == std::string::npos) {
		if (p2 == std::string::npos)
			return std::string(".");
		p = p2;
	} else if (p < p2) {
		p = p2;
	}
	return fname.substr(0, p);
}

bool FSNode::exists(const std::string &fname) const {
	struct stat buf;
	return stat(fname.c_str(), &buf) == 0;
}

const std::string FSNode::get_filename(const std::string &name, const bool return_ext) {
	std::string::size_type p2 = name.rfind('.'), p1 = name.npos;
	if (p2 == name.npos)
		p2 = name.size();

	p1 = name.rfind('/', p2 - 1);
	if (p1 == name.npos) 
		p1 = name.rfind('\\', p2 - 1);
	if (p1 == name.npos)
		p1 = 0;
	else 
		++p1;

	return name.substr(p1, return_ext?std::string::npos: p2 - p1);	
}

const std::string FSNode::get_dir(const std::string &fname) {
	std::string::size_type p = fname.rfind('/');
	if (p == fname.npos)
		throw_ex(("get_dir('%s') failed", fname.c_str()));
	
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

const std::string FSNode::relative_path(const std::string &from_dir, const std::string &to_dir) {
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

const std::string FSNode::normalize(const std::string &path_) {
	std::string path = path_;
	for(size_t i = 0; i < path.size(); ++i) {
		if (path[i] == '\\')
			path[i] = '/';
	}
	std::vector<std::string> p, r;
	mrt::split(p, path, "/");
	for(size_t i = 0; i < p.size(); ++i) {
		if (p[i] == "." || (i > 0 && p[i].empty()))
			continue;
		if (p[i] == ".." && !r.empty()) {
			r.resize(r.size() - 1);
			continue;
		}
		r.push_back(p[i]);
	}
	mrt::join(path, r, "/");
	return path;
}

#ifndef S_ISDIR
#	define S_ISDIR(mode) ((mode) & _S_IFDIR)
#endif

bool FSNode::is_dir(const std::string &name) {
	struct stat buf;
	if (stat(name.c_str(), &buf) != 0)
		return false;
	return S_ISDIR(buf.st_mode) != 0;
}
