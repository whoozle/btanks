#ifndef __BTANKS_SERIALIZATOR_H__
#define __BTANKS_SERIALIZATOR_H__
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

#include <sys/types.h>
#include <string>

namespace mrt {
class Chunk;
class Serializator {
public:
	Serializator();
	Serializator(const mrt::Chunk *chunk);
	~Serializator();
	
	void add(const int n);
	void add(const unsigned int n);
	void add(const float f);
	void add(const std::string &str);
	void add(const bool b);
	void add(const Chunk &c);

	const bool end() const;
	
	void get(int &n) const;
	void get(unsigned int &n) const;
	void get(float &f) const;
	void get(std::string &str) const;
	void get(bool &b) const;
	void get(Chunk &c) const;
	
	const Chunk & getData() const;

	void add(const void *raw, const int size); //same as add(chunk)

protected:
	void get(void *raw, const int size) const; //this one doesnt check anything, just copy next `size` bytes to pointer.

	Chunk *_data;
	mutable size_t _pos;
	bool _owns_data;

private:
	Serializator(const Serializator &s);
	const Serializator & operator=(const Serializator &s);
};
}

#endif
