#ifndef __BTANKS_SERIALIZATOR_H__
#define __BTANKS_SERIALIZATOR_H__

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


#include <sys/types.h>
#include <string>
#include "export_mrt.h"

#include <list>
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
	virtual ~Serializator();
	
	virtual void add(const int n);
	virtual void add(const unsigned int n);
	virtual void add(const float f);
	virtual void add(const std::string &str);
	virtual void add(const bool b);
	virtual void add(const Chunk &c);

	virtual bool end() const;
	
	virtual void get(int &n) const;
	virtual void get(unsigned int &n) const;
	virtual void get(float &f) const;
	virtual void get(std::string &str) const;
	virtual void get(bool &b) const;
	virtual void get(Chunk &c) const;
	
	virtual const Chunk & getData() const;
	virtual void finalize(mrt::Chunk &data); //destroys serializator, but have no overhead on copy. for serialize2 method

	virtual void add(const void *raw, const int size); //same as add(chunk)

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

	//std::list
	
	template <typename T> 
	void add(const std::list<T> &l) {
		add((unsigned)l.size());
		for(typename std::list<T>::const_iterator i = l.begin(); i != l.end(); ++i) 
			add(*i);
	}

	template <typename T>
	void get(std::list<T> &l) const {
		unsigned n; get(n);
		l.resize(n);
		for(typename std::list<T>::iterator i = l.begin(); i != l.end(); ++i) {
			get(*i);
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
		for(typename std::deque<T>::iterator i = q.begin(); i != q.end(); ++i) {
			get(*i);
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
	
	size_t get_current_position() const { return _pos; } //debug only!

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
