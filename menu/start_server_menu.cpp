#include "start_server_menu.h"
#include "sdlx/surface.h"
#include "mrt/logger.h"

StartServerMenu::StartServerMenu() {
	_upper_box.init(500, 80, true);
}


void StartServerMenu::render(sdlx::Surface &dst) {
	_upper_box.render(dst, (dst.getWidth() - _upper_box.w) / 2, 32);
}

bool StartServerMenu::onKey(const SDL_keysym sym) {
	LOG_DEBUG(("key"));
	return false;
}
