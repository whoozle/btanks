#ifndef __BT_LAYER_H__
#define __BT_LAYER_H__

#include "mrt/chunk.h"
#include "sdlx/surface.h"

class Layer {
public:
	sdlx::Surface surface;
	const int impassability;

	Layer(const long w, const long h, const mrt::Chunk & data, const int impassability);
	const long get(const long x, const long y) const;
private: 
	mrt::Chunk _data;
	long _w, _h;
};

#endif

