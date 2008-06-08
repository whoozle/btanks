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

void ImageView::render(sdlx::Surface &surface, const int x, const int y) const {
	Container::render(surface, x, y);
	if (_image == NULL)
		return;
	int mx, my;
	_box->getMargins(mx, my);
	sdlx::Rect clip;

	surface.get_clip_rect(clip);
	surface.set_clip_rect(sdlx::Rect(mx + x, my + y, _w - 2 * mx, _h - 2 * my));
	surface.blit(*_image, x + mx - (int)position.x, y + my - (int)position.y);
	if (_overlay != NULL) 
		surface.blit(*_overlay, x + mx - (int)position.x + _overlay_dpos.x, y + my - (int)position.y + _overlay_dpos.y);
	surface.set_clip_rect(clip);
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
	if (pos.x + w > _image->get_width())
		pos.x = _image->get_width() - w;
	if (pos.y + h > _image->get_height())
		pos.y = _image->get_height() - h;
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
		float dist = destination.distance(position);
		position += math::min(dist, map_vel.length() * (dist > 300?600:(dist < 25?50:dist * 2)) * dt) * map_vel;
	}
}

void ImageView::set_position(const v2<float> &pos) { 
	setDestination(pos);
	position = destination;
}

void ImageView::setDestination(const v2<float> &pos) {
	v2<float> p = pos - v2<float>(_w, _h) / 2;
	if (_overlay)
		p += v2<float>(_overlay->get_width(), _overlay->get_height()) / 2;
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
