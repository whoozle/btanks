#include "image_view.h"
#include "math/binary.h"
#include "sdlx/rect.h"
#include "sdlx/surface.h"
#include "box.h"

ImageView::ImageView(int w, int h) : 
_w(w), _h(h), _image(NULL), _overlay(NULL), 
_box(new Box("menu/background_box.png", _w, _h)) {
	add(0, 0, _box);
}

void ImageView::setOverlay(const sdlx::Surface *overlay, const v2<int> &dpos) {
	_overlay = overlay; 
	_overlay_dpos = dpos;
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
	sdlx::Rect clip;

	surface.getClipRect(clip);
	surface.setClipRect(sdlx::Rect(mx + x, my + y, _w - 2 * mx, _h - 2 * my));
	surface.copyFrom(*_image, x + mx - (int)position.x, y + my - (int)position.y);
	if (_overlay != NULL) 
		surface.copyFrom(*_overlay, x + mx - (int)position.x + _overlay_dpos.x, y + my - (int)position.y + _overlay_dpos.y);
	surface.setClipRect(clip);
}

void ImageView::validate(v2<float> & pos) {
	if (_image == NULL)
		return;
	if (pos.x < 0) 
		pos.x = 0;
		
	if (pos.y < 0)
		pos.y = 0;
	
	int mx, my;
	_box->getMargins(mx, my);
		
	int w = _w - 2 * mx, h = _h - 2 * my;
	if (pos.x + w > _image->getWidth())
		pos.x = _image->getWidth() - w;
	if (pos.y + h > _image->getHeight())
		pos.y = _image->getHeight() - h;
}

void ImageView::tick(const float dt) {
	Container::tick(dt);
	validate(destination);
	validate(position);
	
	v2<float> map_vel = destination - position;
	if (map_vel.quick_length() < 1) {
		position = destination;
	} else {
		map_vel.normalize();
		float dist = math::min(destination.distance(position) / 2, dt * 200);
		position += map_vel * dist;
	}
}

void ImageView::setPosition(const v2<float> &pos) { 
	setDestination(pos);
	position = destination;
}

void ImageView::setDestination(const v2<float> &pos) {
	v2<float> p = pos - v2<float>(_w, _h) / 2;
	if (_overlay)
		p += v2<float>(_overlay->getWidth(), _overlay->getHeight()) / 2;
	destination = p;
}

bool ImageView::onMouseMotion(const int state, const int x, const int y, const int xrel, const int yrel) {
	if ((state & SDL_BUTTON(1)) == 0)
		return false;
	
	position.x -= xrel; 
	position.y -= yrel; 
	validate(position);
	destination = position;
	return true;
}
