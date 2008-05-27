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
		screenshot = ResourceManager->loadSurface("../" + fname);
	} else 
		throw_ex(("no disabled version of the screenshot found"));
}

void DisabledVideoControl::render(sdlx::Surface &surface, const int x, const int y) const {
	surface.copyFrom(*screenshot, x, y);
}

void DisabledVideoControl::getSize(int &w, int &h) const {
	w = screenshot->getWidth();
	h = screenshot->getHeight();
}
