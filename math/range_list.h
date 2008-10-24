#ifndef BTANKS_MATH_RANGE_LIST_H__
#define BTANKS_MATH_RANGE_LIST_H__

#include <map>

template<typename T>
class range_list : public std::map<const T, T> {
public: 
	typedef std::map<const T, T> parent_type;

private: 
	typename parent_type::iterator pack_left(typename parent_type::iterator i) {
		if (i == parent_type::begin())
			return i;

		typename parent_type::iterator prev = i;
		--prev;

		if (prev->second + 1 < i->first)
			return i;
		
		T e = i->second;
		
		parent_type::erase(i);
		prev->second = e;

		return pack_left(prev);
	}

	typename parent_type::iterator pack_right(typename parent_type::iterator i) {
		if (i == parent_type::end())
			return i;

		typename parent_type::iterator next = i;
		++next;
		if (next == parent_type::end())
			return i;
		
		if (i->second + 1 < next->first)
			return i;

		T e = next->second;
		parent_type::erase(next);
		i->second = e;

		return pack_right(i);
	}

public: 
	void insert(const T& value) {
		if (parent_type::empty()) {
			parent_type::insert(typename parent_type::value_type(value, value));
			return;
		}
	
		typename parent_type::iterator i = lower_bound(value);
		if (i != parent_type::end()) {
			if (i->first == value)
				return;

			if (value + 1 == i->first) {
				T e = i->second;
				erase(i);
				i = parent_type::insert(typename parent_type::value_type(value, e)).first; //expand beginning
				i = pack_left(i);
			}
		}
		
		if (i != parent_type::begin())
			--i; //look for the previous interval

		if (i->first <= value && value <= i->second) //included in interval
			return;
		
		if (i->second + 1 == value) {
			i->second = value;
			i = pack_right(i);
			return;
		}
		
		parent_type::insert(typename parent_type::value_type(value, value));
	}
};

#endif

