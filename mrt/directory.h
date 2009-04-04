#ifndef __MRT_DIRECTORY_H__
#define __MRT_DIRECTORY_H__

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


#ifndef _WINDOWS
#	include <sys/types.h>
#	include <dirent.h>
#endif
#include "base_directory.h"
#include "fs_node.h"

namespace mrt { 

class MRTAPI Directory : public BaseDirectory, public FSNode {
public: 

	Directory();
	virtual void create(const std::string &path, const bool recurse = false);
	virtual void open(const std::string &path);
	virtual bool opened() const;
	virtual const std::string read() const;
	virtual void close();
	virtual ~Directory();
	
	static const std::string get_home();
	static const std::string get_app_dir(const std::string &name, const std::string &shortname);
	
private: 

#ifdef _WINDOWS
	typedef long dir_t;
	mutable std::string _first_value;
#else
	typedef DIR * dir_t;
#endif

	dir_t _handle;
};
}

#endif

