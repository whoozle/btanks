#include "layer.h"
#include <assert.h>

Layer::Layer(const long w, const long h, const mrt::Chunk & data) : _w(w), _h(h) {
	_data = data;
}

const long Layer::get(const long x, const long y) const {
	if (x >= _w || y >= _h) 
		return 0;
	
	register long * ptr = (long *) _data.getPtr();
	return *(ptr + _w * y + x);
}
