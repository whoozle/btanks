/* M-runtime for c++
 * Copyright (C) 2005-2008 Vladimir Menshakov
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

#include "SDL/SDL_syswm.h"
#include "directory.h"
#include "ioexception.h"
#include <errno.h>
#include <stdlib.h>

using namespace mrt;

Directory::Directory() : _handle(0) {}
Directory::~Directory() {
	close();
}

const bool Directory::opened() const {
	return _handle != 0;
}

#ifndef WIN32

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

const std::string Directory::getHome() {
	const char *home_env = getenv("HOME");
	if (home_env == NULL) 
		throw_ex(("getting home directory now is possible only via HOME variable. fix it if you want."));
	return home_env;
}

const std::string Directory::getAppDir(const std::string &name, const std::string &shortname) {
	std::string path = getHome() + "/." + shortname;
	if (!exists(path)) 
		create(path);
	return path;
}

void Directory::create(const std::string &path) {
	if (mkdir(path.c_str(), S_IRUSR | S_IWUSR | S_IXUSR) == -1)
		throw_io(("mkdir"));
}


#else 
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

#include <shlobj.h>

const std::string Directory::getAppDir(const std::string &name, const std::string &shortname) {
	std::string path = getHome() + "\\" + name;
	if (!exists(path)) 
		create(path);
	return path;	
}

const std::string Directory::getHome() {
    SDL_SysWMinfo   info;
	HWND hwnd = NULL;
/*
    SDL_VERSION(&info.version);
    if (SDL_GetWMInfo(&info) != -1)
        hwnd = info.window;
*/
    TCHAR path[MAX_PATH];
   	if (FAILED(::SHGetFolderPath(hwnd, CSIDL_APPDATA | CSIDL_FLAG_CREATE, NULL, SHGFP_TYPE_CURRENT, path)))
   		throw_ex(("GetFolderPath(CSIDL_APPDATA|CSIDL_FLAG_CREATE) failed"));
	return path;
}


void Directory::create(const std::string &path) {
	::CreateDirectory(path.c_str(), NULL);
}

#endif
