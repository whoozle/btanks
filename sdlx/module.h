#ifndef SDLX_MODULE_H__
#define SDLX_MODULE_H__

/* sdlx - c++ wrapper for libSDL
 * Copyright (C) 2005-2007 Vladimir Menshakov
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

#include "export_sdlx.h"
#include <string>

namespace sdlx {

class SDLXAPI Module {
public: 
	static const std::string mangle(const std::string &name);

	Module();
	void load(const std::string &name);
	void *sym(const std::string &name) const;
	void leak();
	void unload();
	~Module();
private:
	void * handle;	
};

}

#endif

