#include "layer.h"
#include <assert.h>

Layer::Layer(const int w, const int h, const mrt::Chunk & data, const int impassability, const bool pierceable) : impassability(impassability), pierceable(pierceable), _w(w), _h(h){
	_data = data;
}
