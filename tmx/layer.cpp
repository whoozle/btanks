#include "layer.h"
#include <assert.h>

Layer::Layer(const int w, const int h, const mrt::Chunk & data, const int impassability) : impassability(impassability), _w(w), _h(h){
	_data = data;
}
