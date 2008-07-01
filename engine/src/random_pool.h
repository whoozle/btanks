#ifndef BTANKS_RANDOM_POOL_H__
#define BTANKS_RANDOM_POOL_H__

#include <deque>
#include <assert.h>
#include "mrt/random.h"

template<typename T>
class RandomPool {
public: 
	RandomPool() : min(0), max(0), step(0) {}
	void init(T min_, T max_, T step_ = 1) {
		min = min_; max = max_; step = step_;
		hash();
	}
	
	T get() {
		if (pool.empty())
			hash();

		assert(!pool.empty());
		int idx = mrt::random(pool.size());
		typename container_t::iterator i = pool.begin();
		std::advance(i, idx);
		T r = *i;
		pool.erase(i);
		return r;
	}
	
	void hash() {
		assert(max != min); //you missed pool initialization
		pool.clear();
		for(T i = min; i < max; i += step) 
			pool.push_back(i);
	}

protected: 
	T min, max, step;
	typedef std::deque<T> container_t;
	container_t pool;
};

#endif

