#ifndef __BTANKS_UTILS_H__
#define __BTANKS_UTILS_H__

#include <algorithm>

template <class T> struct delete_ptr2 : public std::unary_function<T, void> {
	void operator()(T &x) {
		delete x.second;
		x.second = NULL;
	}
};

#endif

