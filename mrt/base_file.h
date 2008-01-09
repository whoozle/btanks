#ifndef __MRT_BASEFILE__
#define __MRT_BASEFILE__

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
#include "fs_node.h"
#include "export_mrt.h"

namespace mrt {

class Chunk;

class MRTAPI BaseFile {
public: 
	virtual ~BaseFile();

	void readAll(std::string &str) const;
	void readAll(Chunk &ch) const;
	void writeAll(const Chunk &ch) const;
	void writeAll(const std::string &str) const;

	virtual void open(const std::string &fname, const std::string &mode) = 0;
	virtual const bool opened() const = 0;
	
	virtual int seek(long offset, int whence) const = 0;
	virtual long tell() const = 0;
	virtual void write(const Chunk &ch) const = 0;

	virtual const off_t getSize() const = 0;
	virtual const size_t read(void *buf, const size_t size) const = 0;
	virtual void close();
	
	virtual const bool eof() const = 0;

	virtual const bool readLine(std::string &str, const size_t bufsize = 1024) const = 0;
};

}
#endif
