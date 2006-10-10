#ifndef __BTANKS_MINMAX_H__
#define __BTANKS_MINMAX_H__

namespace math {

template <typename T>
	const T max(const T a, const T b) {
		return (a > b)?a:b;
	}

template <typename T>
	const T min(const T a, const T b) {
		return (a < b)?a:b;
	}
}

#endif

