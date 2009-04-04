#ifndef __MRT_BASE_DIRECTORY_H__
#define __MRT_BASE_DIRECTORY_H__

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

#include <string>
#include "export_mrt.h"

namespace mrt { 

class MRTAPI BaseDirectory {
public: 
	virtual void open(const std::string &path) = 0;
	virtual bool opened() const = 0;
	virtual const std::string read() const = 0;
	virtual void close() = 0;
	virtual void create(const std::string &path, const bool recurse = false) = 0;
	virtual ~BaseDirectory() = 0;
};
}

#endif

