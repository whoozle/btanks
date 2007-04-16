#ifndef __MRT_BASEFILE__
#define __MRT_BASEFILE__
/* M-Runtime for c++
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

#include <string>
#include <stdio.h>
#include "fs_node.h"
#include "export_mrt.h"

namespace mrt {

class Chunk;

class MRTAPI File : public FSNode {
public: 
	File();
	void open(const std::string &fname, const std::string &mode);
	
	int seek(long offset, int whence);
	void write(const Chunk &ch) const;

	void readAll(Chunk &ch) const;
	void writeAll(const Chunk &ch) const;
	void writeAll(const std::string &str) const;

	const bool readLine(std::string &str) const;
	const off_t getSize() const;
	const size_t read(void *buf, const size_t size) const;
	void close();
	
	const bool eof() const;
	inline operator FILE*() { return _f; }
	FILE * unlink(); //unlinks FILE* structure from this object
private: 
	FILE *_f;
};

}

#endif
