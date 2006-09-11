#ifndef __BTANKS_ABS_H__
#define __BTANKS_ABS_H__

namespace math {

template<typename T> 
	inline T abs(const T v) {
			return (v < 0) ? -v: v;
		}
}

#endif

