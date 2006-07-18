#ifndef __BT_LAYER_H__
#define __BT_LAYER_H__

#include "mrt/chunk.h"
#include "sdlx/surface.h"

class Layer {
public:
	sdlx::Surface surface;
	const float impassability;

	Layer(const long w, const long h, const mrt::Chunk & data, const float impassability);
	const long get(const long x, const long y) const;
private: 
	mrt::Chunk _data;
	long _w, _h;
};

#endif

