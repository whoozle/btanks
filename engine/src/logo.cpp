#include "logo.h"
#include "sdlx/surface.h"

Logo::Logo(sdlx::Surface *surface, float t, Uint32 color) : logo(surface), t(t), color(color) {}


void Logo::render(const float dt, sdlx::Surface &surface) {
	surface.fill(color);
	surface.blit(*logo, 
		(surface.get_width() - logo->get_width()) / 2, 
		(surface.get_height() - logo->get_height()) / 2
	);
	t -= dt;
}

Logo::~Logo() {
	delete logo;
}
