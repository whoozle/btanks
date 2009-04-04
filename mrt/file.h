#ifndef __MRT_FILE_H__
#define __MRT_FILE_H__

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
#include <stdio.h>
#include "base_file.h"
#include "fs_node.h"
#include "export_mrt.h"

namespace mrt {

class Chunk;

class MRTAPI File : public BaseFile, public FSNode {
public: 
	bool readline(std::string &str, const size_t bufsize = 1024) const;

	File();
	~File();

	virtual void open(const std::string &fname, const std::string &mode);
	virtual bool opened() const;
	
	virtual void seek(long offset, int whence) const;
	virtual long tell() const;
	virtual void write(const Chunk &ch) const;

	virtual const off_t get_size() const;
	virtual const size_t read(void *buf, const size_t size) const;
	virtual void close();
	
	virtual bool eof() const;

	inline operator FILE*() const { return _f; }
	FILE * unlink(); //unlinks FILE* structure from this object
private: 
	FILE *_f;
};

}

#endif
