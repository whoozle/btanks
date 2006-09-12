#ifndef __BT_LAYER_H__
#define __BT_LAYER_H__

#include "mrt/chunk.h"
#include "sdlx/surface.h"

class Layer {
public:
	sdlx::Surface surface;
	const int impassability;
	const bool pierceable;

	Layer(const int w, const int h, const mrt::Chunk & data, const int impassability, const bool pierceable);

	inline const int get(const int x, const int y) const {
		if (x < 0 || x >= _w || y < 0 || y >= _h) 
			return 0;	
		return *((int *) _data.getPtr() + _w * y + x);
	}

private: 
	mrt::Chunk _data;
	int _w, _h;
};

#endif

