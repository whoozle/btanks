#include "image.h"
#include "sdlx/surface.h"

Image::Image(const sdlx::Surface *image) : image(image) {
}

void Image::set(const sdlx::Surface *image) {
	this->image = image;
}

void Image::render(sdlx::Surface &surface, const int x, const int y) const {
	if (image == NULL)
		return;
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
