#include "image_view.h"
#include "math/binary.h"
#include "sdlx/rect.h"
#include "sdlx/surface.h"
#include "box.h"

ImageView::ImageView(int w, int h) : _w(w), _h(h), _image(NULL), _box(new Box("menu/background_box.png", _w, _h)) {
	add(0, 0, _box);
}

void ImageView::init(const sdlx::Surface *image) {
	_image = image;
}

void ImageView::render(sdlx::Surface &surface, const int x, const int y) {
	Container::render(surface, x, y);
	if (_image == NULL)
		return;
	int mx, my;
	_box->getMargins(mx, my);
	surface.copyFrom(*_image, sdlx::Rect((int)position.x, (int)position.y, _w - 2 * mx, _h - 2 * my), x + mx, y + my);
}

void ImageView::tick(const float dt) {
	Container::tick(dt);
	v2<float> map_vel = destination - position;
	if (map_vel.quick_length() < 1) {
		position = destination;
	} else {
		map_vel.normalize();
		float dist = math::min(destination.distance(position), dt * 200);
		position += map_vel * dist;
	}
}
