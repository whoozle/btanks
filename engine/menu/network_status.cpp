#include "network_status.h"
#include "resource_manager.h"

NetworkStatusControl::NetworkStatusControl() : 
	Tooltip("menu", "network-status", true), _bclose(ResourceManager->loadSurface("menu/disconnect.png")) {
}

void NetworkStatusControl::render(sdlx::Surface &surface, const int x, const int y) const {
	Tooltip::render(surface, x, y);
	int mx, my, w, h;
	_background.getMargins(mx, my);
	_background.getSize(w, h);

	_close_area.w = _bclose->getWidth();
	_close_area.h = _bclose->getHeight();
	_close_area.x = w - mx - _close_area.w;
	_close_area.y = h - my - _close_area.h;

	surface.copyFrom(*_bclose, _close_area.x, _close_area.y);
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
