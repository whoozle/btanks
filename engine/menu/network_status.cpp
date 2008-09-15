#include "network_status.h"
#include "resource_manager.h"

NetworkStatusControl::NetworkStatusControl() : 
	Tooltip("menu", "network-status", true), _bclose(NULL) {
}

void NetworkStatusControl::render(sdlx::Surface &surface, const int x, const int y) const {
	AUTOLOAD_SURFACE(_bclose, "menu/disconnect.png");

	Tooltip::render(surface, x, y);
	int mx, my, w, h;
	_background.getMargins(mx, my);
	_background.get_size(w, h);

	_close_area.w = _bclose->get_width();
	_close_area.h = _bclose->get_height();
	_close_area.x = w - mx - _close_area.w;
	_close_area.y = h - my - _close_area.h;

	surface.blit(*_bclose, _close_area.x, _close_area.y);
}

bool NetworkStatusControl::onMouse(const int button, const bool pressed, const int x, const int y) {
	if (_close_area.in(x, y)) {
		//LOG_DEBUG(("%d %d", x, y));
		if (!pressed)
			invalidate();
		return true;
	}
	return false;
}
