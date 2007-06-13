#include "image_view.h"
#include "math/binary.h"
#include "sdlx/rect.h"
#include "sdlx/surface.h"

ImageView::ImageView(int w, int h) : _w(w), _h(h), _image(NULL)  {}

void ImageView::init(const sdlx::Surface *image) {
	_image = image;
}

void ImageView::render(sdlx::Surface &surface, const int x, const int y) {
	if (_image == NULL)
		return;
	
	surface.copyFrom(*_image, sdlx::Rect((int)position.x, (int)position.y, _w, _h), x, y);
}

void ImageView::getSize(int &w, int &h) const {
	w = _w; h = _h;
}

void ImageView::tick(const float dt) {
	v2<float> map_vel = destination - position;
	if (map_vel.quick_length() < 1) {
		position = destination;
	} else {
		map_vel.normalize();
		float dist = math::min(destination.distance(position), dt * 200);
		position += map_vel * dist;
	}
}
