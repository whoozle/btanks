#ifndef __BTANKS_DICT_SERIALIZATOR_H__
#define __BTANKS_DICT_SERIALIZATOR_H__

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

#include "serializator.h"

namespace mrt {

class DLLEXPORT DictionarySerializator : public Serializator {
public:
	DictionarySerializator();
	DictionarySerializator(const mrt::Chunk *chunk);
	~DictionarySerializator();
	
	virtual void add(const std::string &str);
	virtual void get(std::string &str) const;

	virtual void finalize(mrt::Chunk &data); //destroys serializator, but have no overhead on copy. for serialize2 method

private:
	DictionarySerializator(const DictionarySerializator &s);
	const DictionarySerializator & operator=(const DictionarySerializator &s);

	const Chunk & getData() const;
	void read_dict();
	
	int next_id;
	typedef std::map<const std::string, int> Dict;
	typedef std::map<const int, std::string> RDict;
	Dict dict;
	RDict rdict;
};

}

#endif
