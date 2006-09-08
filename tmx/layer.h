#ifndef __BT_LAYER_H__
#define __BT_LAYER_H__

#include "mrt/chunk.h"
#include "sdlx/surface.h"

class Layer {
public:
	sdlx::Surface surface;
	const int impassability;

	Layer(const long w, const long h, const mrt::Chunk & data, const int impassability);

	inline const long get(const long x, const long y) const {
		if (x < 0 || x >= _w || y < 0 || y >= _h) 
			return 0;	
		return *((long *) _data.getPtr() + _w * y + x);
	}

private: 
	mrt::Chunk _data;
	long _w, _h;
};

#endif

