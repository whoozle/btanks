#include "image.h"
#include "sdlx/surface.h"

Image::Image(const sdlx::Surface *image) : image(image) {
}

void Image::set_source(const sdlx::Rect &rect) {
	src = rect;
}

void Image::set(const sdlx::Surface *image) {
	this->image = image;
}

void Image::render(sdlx::Surface &surface, const int x, const int y) const {
	if (image == NULL)
		return;

	if (src.w != 0) 
		surface.blit(*image, src, x, y);
	else
		surface.blit(*image, x, y);
}

void Image::get_size(int &w, int &h) const {
	if (image == NULL) {
		w = h = 0;
	} else {
		w = image->get_width();
		h = image->get_height();
	}
}
