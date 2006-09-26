#include "layer.h"
#include <assert.h>
#include "mrt/exception.h"

Layer::Layer(const int w, const int h, const mrt::Chunk & data, const int impassability, const bool pierceable) : impassability(impassability), pierceable(pierceable), _w(w), _h(h){
	_data = data;
}

void Layer::optimize(std::vector<sdlx::Surface *> & tilemap) {
	int size =  _data.getSize();
	_data_s.setSize(size);
	size /= 4;
	int *ptr = (int *)_data.getPtr();
	sdlx::Surface **s_ptr = (sdlx::Surface **) _data_s.getPtr();
	while(size--) {
		int tid = *ptr++;

		if (tid == 0) { 
			*s_ptr++ = 0;
		} else {
			if ((unsigned)tid >= tilemap.size())
				throw_ex(("got invalid tile id %d", tid));
			*s_ptr++ = tilemap[tid];
		}
	}
}
