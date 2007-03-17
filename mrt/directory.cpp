/* M-runtime for c++
 * Copyright (C) 2005-2007 Vladimir Menshakov
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
	if (home_env != NULL)
		return home_env;
	throw_ex(("getting home directory now is possible only via HOME variable. fix it if you want."));
	return std::string();
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

const std::string Directory::getHome() {
	throw_ex(("implement me"));
	return std::string();
}

#endif
