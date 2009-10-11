#include "logo.h"
#include "sdlx/surface.h"

Logo::Logo(sdlx::Surface *surface, float d, Uint32 color) : logo(surface), duration(d), t(0), color(color) {}


void Logo::render(const float dt, sdlx::Surface &surface) {
	surface.fill(color);
	if (t < 1) {
		logo->set_alpha((int)(255 * t));
	} else
		logo->set_alpha(255);

	float r = duration - t;
	if (r < 1) {
		logo->set_alpha((int)(255 * r));
	}

	surface.blit(*logo, 
		(surface.get_width() - logo->get_width()) / 2, 
		(surface.get_height() - logo->get_height()) / 2
	);

	t += dt;
}

Logo::~Logo() {
	delete logo;
}
