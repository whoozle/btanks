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

#include "directory.h"
#include "ioexception.h"
#include <errno.h>
#include <stdlib.h>

using namespace mrt;

Directory::Directory() : _handle(0) {}
Directory::~Directory() {
	close();
}

bool Directory::opened() const {
	return _handle != 0;
}

#ifndef _WINDOWS

#include <sys/types.h>
#include <sys/stat.h>
		  

void Directory::open(const std::string &path) {
	close();
	if (path.empty())
		throw_ex(("Directory::open called with empty path"));
	_handle = opendir(path.c_str());
	if (_handle == NULL)
		throw_io(("opendir('%s')", path.c_str()));
}

const std::string Directory::read() const {
	if (_handle == NULL)
		throw_ex(("Directory::read called on uninitialized object"));
	struct dirent *dir = readdir(_handle);
	if (dir == NULL) {
		//if (errno) 
		//	throw_io(("readdir"));
		return std::string();
	}
	return dir->d_name;
}

void Directory::close() {
	if (_handle != NULL) {
		closedir(_handle);
		_handle = NULL;
	}
}

void Directory::create(const std::string &path, const bool recurse) {
	if (!recurse) {
		if (mkdir(path.c_str(), S_IRUSR | S_IWUSR | S_IXUSR) == -1)
			throw_io(("mkdir"));
	} else {
		//LOG_DEBUG(("create(%s, true)", path.c_str()));
		std::string p = normalize(path);
		//LOG_DEBUG(("normalized path: %s", p.c_str()));
		if (p.empty())
			return;
		std::vector<std::string> res;
		mrt::split(res, p, "/");
		if (res.empty())
			return;

		p = res[0];
		
		//LOG_DEBUG(("creating directory: %s", p.c_str()));
		mkdir(p.c_str(), S_IRUSR | S_IWUSR | S_IXUSR);
		for(size_t i = 1; i < res.size(); ++i) {
			p += "/";
			p += res[i];
			//LOG_DEBUG(("creating directory: %s", p.c_str()));
			mkdir(p.c_str(), S_IRUSR | S_IWUSR | S_IXUSR);
		}
	}
}


#else 
#include <windows.h>
#include <io.h>

void Directory::open(const std::string &path) {
	struct _finddata_t filedata;
	if ((_handle = _findfirst((path + "/*").c_str(), &filedata)) == -1) {
		_handle = 0;
		throw_io(("findfirst('%s')", path.c_str()));
	}
   	_first_value = filedata.name;
}

const std::string Directory::read() const {
	if (!_first_value.empty()) {
		std::string r = _first_value;
		_first_value.clear();
		return r;
	}
	struct _finddata_t filedata;
	if (_findnext(_handle, &filedata) == 0) {
		return filedata.name;
	}
	return std::string();
}

void Directory::close() {
	if (_handle == 0)
		return;

	_findclose(_handle);
	_handle = 0;
}

void Directory::create(const std::string &path, const bool recurse) {
	if (!recurse) {
		::CreateDirectory(path.c_str(), NULL);
	} else {
		//LOG_DEBUG(("create(%s, true)", path.c_str()));
		std::string p = normalize(path);
		//LOG_DEBUG(("normalized path: %s", p.c_str()));
		if (p.empty())
			return;
		std::vector<std::string> res;
		mrt::split(res, p, "/");
		if (res.empty())
			return;

		p = res[0];
		
		//LOG_DEBUG(("creating directory: %s", p.c_str()));
		::CreateDirectory(p.c_str(), NULL);
		for(size_t i = 1; i < res.size(); ++i) {
			p += "/";
			p += res[i];
			//LOG_DEBUG(("creating directory: %s", p.c_str()));
			::CreateDirectory(p.c_str(), NULL);
		}
	}
}

#endif


#ifdef _WINDOWS
#include <shlobj.h>

const std::string Directory::get_app_dir(const std::string &name, const std::string &shortname) {
	std::string path = get_home() + "\\" + name;
	mrt::Directory dir;
	try {
		dir.create(path);
	} catch(...) {}
	return path;	
}

const std::string Directory::get_home() {
	HWND hwnd = NULL;
/*
    SDL_SysWMinfo   info;
    SDL_VERSION(&info.version);
    if (SDL_GetWMInfo(&info) != -1)
        hwnd = info.window;
*/
    TCHAR path[MAX_PATH];
   	if (FAILED(::SHGetFolderPath(hwnd, CSIDL_APPDATA | CSIDL_FLAG_CREATE, NULL, SHGFP_TYPE_CURRENT, path)))
   		throw_ex(("GetFolderPath(CSIDL_APPDATA|CSIDL_FLAG_CREATE) failed"));
	return path;
}

#else 

const std::string Directory::get_home() {
	const char *home_env = getenv("HOME");
	if (home_env == NULL) 
		throw_ex(("getting home directory now is possible only via HOME variable. fix it if you want."));
	return home_env;
}

const std::string Directory::get_app_dir(const std::string &name, const std::string &shortname) {
	std::string path = get_home() + "/." + shortname;
	mrt::Directory dir;
	try {
		dir.create(path);
	} catch(...) {}
	return path;
}

#endif

