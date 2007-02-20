#include "scroll_list.h"

ScrollList::ScrollList(const int w, const int h) {
	_background.init("menu/background_box.png", w, h);
}

void ScrollList::render(sdlx::Surface &surface, const int x, const int y) {
	_background.render(surface, x, y);
}
bool ScrollList::onKey(const SDL_keysym sym) {
	return false;
}
bool ScrollList::onMouse(const int button, const bool pressed, const int x, const int y) {
	return false;
}
