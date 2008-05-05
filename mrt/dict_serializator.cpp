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
#include "dict_serializator.h"
#include "exception.h"
#include "logger.h"
#include "chunk.h"

using namespace mrt; 

DictionarySerializator::DictionarySerializator() : Serializator(), next_id(0) {}

DictionarySerializator::DictionarySerializator(const mrt::Chunk *chunk) : Serializator(chunk), next_id(0) {
	read_dict();
}

void DictionarySerializator::read_dict() {
	unsigned n;
	Serializator::get(n);
	LOG_DEBUG(("dictionary:  %u items", n));
	std::string str;
	int id;
	while(n--) {
		Serializator::get(str);
		Serializator::get(id);
		rdict.insert(RDict::value_type(id, str));
	}
}

DictionarySerializator::~DictionarySerializator() {}
	
void DictionarySerializator::add(const std::string &str) {
	int id;
	Dict::iterator i = dict.find(str);
	if (i != dict.end()) {
		id = i->second;
	} else {
		dict.insert(Dict::value_type(str, id = next_id++));
	}
	Serializator::add(id);
}

void DictionarySerializator::get(std::string &str) const {
	int id;
	Serializator::get(id);
	RDict::const_iterator i = rdict.find(id);
	if (i == rdict.end())
		throw_ex(("string with id %d was not found in dictionary", id));
	str = i->second;
}

const Chunk & DictionarySerializator::getData() const {
	throw_ex(("use finalize instead"));
}

void DictionarySerializator::finalize(mrt::Chunk &data) {
	LOG_DEBUG(("finalize: %u items in dictionary", (unsigned)dict.size()));

	mrt::Serializator s;
	s.add(dict);
	s.finalize(data);
	
	mrt::Chunk stub;
	Serializator::finalize(stub);
	data.append(stub); //optimize it
}
