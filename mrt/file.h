#ifndef __MRT_FILE_H__
#define __MRT_FILE_H__

/* M-Runtime for c++
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

#include <string>
#include <stdio.h>
#include "base_file.h"
#include "fs_node.h"
#include "export_mrt.h"

namespace mrt {

class Chunk;

class MRTAPI File : public BaseFile, public FSNode {
public: 
	const bool readLine(std::string &str, const size_t bufsize = 1024) const;

	File();

	virtual void open(const std::string &fname, const std::string &mode);
	virtual const bool opened() const;
	
	virtual int seek(long offset, int whence) const;
	virtual long tell() const;
	virtual void write(const Chunk &ch) const;

	virtual const off_t getSize() const;
	virtual const size_t read(void *buf, const size_t size) const;
	virtual void close();
	
	virtual const bool eof() const;

	inline operator FILE*() { return _f; }
	FILE * unlink(); //unlinks FILE* structure from this object
private: 
	FILE *_f;
};

}

#endif
