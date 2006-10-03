#ifndef __BT_LAYER_H__
#define __BT_LAYER_H__

#include "mrt/chunk.h"
#include "sdlx/surface.h"
#include <vector>

#define PRERENDER_LAYERS
#undef PRERENDER_LAYERS


class Layer {
public:
#ifdef PRERENDER_LAYERS
	sdlx::Surface surface;
#endif
	const int impassability;
	const bool pierceable;

	Layer(const int w, const int h, const mrt::Chunk & data, const int impassability, const bool pierceable);

	inline const int get(const int x, const int y) const {
		if (x < 0 || x >= _w || y < 0 || y >= _h) 
			return 0;	
		return *((int *) _data.getPtr() + _w * y + x);
	}

	inline const sdlx::Surface* getSurface(const int x, const int y) const {
		if (x < 0 || x >= _w || y < 0 || y >= _h) 
			return NULL;
		return *((sdlx::Surface **) _data_s.getPtr() + _w * y + x);
	}

	void optimize(std::vector<sdlx::Surface *> & tilemap);

private: 
	mrt::Chunk _data;
	mrt::Chunk _data_s;
	int _w, _h;
};

#endif

