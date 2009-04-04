#ifndef __MRT_BASEFILE__
#define __MRT_BASEFILE__

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

class Chunk;

class MRTAPI BaseFile {
public: 
	virtual ~BaseFile();

	void read_all(std::string &str) const;
	void read_all(Chunk &ch) const;
	void write_all(const Chunk &ch) const;
	void write_all(const std::string &str) const;

	virtual void open(const std::string &fname, const std::string &mode) = 0;
	virtual bool opened() const = 0;
	
	virtual void seek(long offset, int whence) const = 0;
	virtual long tell() const = 0;
	virtual void write(const Chunk &ch) const = 0;

	virtual const off_t get_size() const = 0;
	virtual const size_t read(void *buf, const size_t size) const = 0;
	virtual void close();
	
	virtual bool eof() const = 0;

	virtual bool readline(std::string &str, const size_t bufsize = 1024) const; //default 1-byte read implementation

	void readLE16(int &x) const;
	void readLE32(int &x) const;
	void readLE16(unsigned int &x) const;
	void readLE32(unsigned int &x) const;

};

}
#endif
