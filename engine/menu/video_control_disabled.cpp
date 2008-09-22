#include "video_control_disabled.h"
#include "resource_manager.h"
#include "finder.h"
#include "sdlx/sdl_ex.h"
#include "sdlx/surface.h"

DisabledVideoControl::DisabledVideoControl(const std::string &base, const std::string &name) : 
screenshot(NULL)
{
	std::string fname = "maps/" + name + "_disabled.jpg";
	if (Finder->exists(base, fname)) {
		screenshot = ResourceManager->load_surface("../" + fname);
	} else 
		throw_ex(("no disabled version of the screenshot found"));
}

void DisabledVideoControl::render(sdlx::Surface &surface, const int x, const int y) const {
	surface.blit(*screenshot, x, y);
}

void DisabledVideoControl::get_size(int &w, int &h) const {
	w = screenshot->get_width();
	h = screenshot->get_height();
}
