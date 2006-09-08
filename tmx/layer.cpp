#include "layer.h"
#include <assert.h>

Layer::Layer(const long w, const long h, const mrt::Chunk & data, const int impassability) : impassability(impassability), _w(w), _h(h){
	_data = data;
}
