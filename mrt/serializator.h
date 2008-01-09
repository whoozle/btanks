#ifndef __BTANKS_SERIALIZATOR_H__
#define __BTANKS_SERIALIZATOR_H__

/* M-runtime for c++
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

#include <sys/types.h>
#include <string>
#include "export_mrt.h"

#include <deque>
#include <map>
#include <set>
#include <vector>

namespace mrt {
class Chunk;
class DLLEXPORT Serializator {
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

	//add/get for std containers
	
	//std::set

	template <typename T> 
	void add(const std::set<T> &s) {
		add((unsigned)s.size());
		for(typename std::set<T>::const_iterator i = s.begin(); i != s.end(); ++i) 
			add(*i);
	}

	template <typename T>
	void get(std::set<T> &s) const {
		s.clear();
		unsigned n; get(n);
		T v;
		while(n--) {
			get(v);
			s.insert(v);
		}
	}

	//std::deque
	
	template <typename T> 
	void add(const std::deque<T> &q) {
		add((unsigned)q.size());
		for(typename std::deque<T>::const_iterator i = q.begin(); i != q.end(); ++i) 
			add(*i);
	}

	template <typename T>
	void get(std::deque<T> &q) const {
		unsigned n; get(n);
		q.resize(n);
		for(unsigned i = 0; i < n; ++i) {
			get(q[i]);
		}
	}

	//std::vector, the same as deque

	template <typename T> 
	void add(const std::vector<T> &q) {
		add((unsigned)q.size());
		for(typename std::vector<T>::const_iterator i = q.begin(); i != q.end(); ++i) 
			add(*i);
	}

	template <typename T>
	void get(std::vector<T> &q) const {
		unsigned n; get(n);
		q.resize(n);
		for(unsigned i = 0; i < n; ++i) {
			get(q[i]);
		}
	}

	//std::map
	
	template <typename T1, typename T2>
	void add(const std::map<const T1, T2> &m) {
		add((unsigned)m.size());
		for(typename std::map<const T1, T2>::const_iterator i = m.begin(); i != m.end(); ++i) {
			add(i->first);
			add(i->second);
		}
	}

	template <typename T1, typename T2>
	void get(std::map<const T1, T2> &m) const {
		m.clear();
		unsigned n;
		get(n);
		T1 key;
		T2 value;
		while(n--) {
			get(key);
			get(value);
			m.insert(typename std::map<const T1, T2>::value_type(key, value));
		}
	}


	template <class T> void add(const T& t) { t.serialize(*this); }
	template <class T> void get(T& t) const { t.deserialize(*this); }

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
