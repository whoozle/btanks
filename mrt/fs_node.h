#ifndef __MRT_FS_NODE_H__
#define __MRT_FS_NODE_H__

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

class MRTAPI FSNode {
public:
	virtual ~FSNode() {}
	virtual bool exists(const std::string &fname) const;
	static const std::string get_dir(const std::string &fname);
	static const std::string get_parent_dir(const std::string &fname);
	static const std::string get_filename(const std::string &fname, const bool return_ext = true);
	static const std::string relative_path(const std::string &from_dir, const std::string &to_dir);
	static const std::string normalize(const std::string &path);
	static bool is_dir(const std::string &name);
};

}

#endif
