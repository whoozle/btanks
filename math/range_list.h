#ifndef BTANKS_MATH_RANGE_LIST_H__
#define BTANKS_MATH_RANGE_LIST_H__

#include <map>

template<typename T>
class range_list : public std::map<const T, T> {
public: 
	typedef std::map<const T, T> parent_type;

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
				parent_type::insert(typename parent_type::value_type(value, e)); //expand beginning
				return;
			}
		}
		
		if (i != parent_type::begin())
			--i; //look for the previous interval

		if (i->first <= value && value <= i->second) //included in interval
			return;
		
		if (i->second + 1 == value) {
			i->second = value;
			return;
		}
		
		parent_type::insert(typename parent_type::value_type(value, value));
	}
};

#endif

