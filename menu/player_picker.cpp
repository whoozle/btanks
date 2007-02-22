#include "player_picker.h"

PlayerPicker::PlayerPicker(const int w, const int h) {
	_background.init("menu/background_box.png", w, h);
}

void PlayerPicker::render(sdlx::Surface &surface, const int x, const int y) {
	_background.render(surface, x, y);
}

bool PlayerPicker::onKey(const SDL_keysym sym) {
	return false;
}

bool PlayerPicker::onMouse(const int button, const bool pressed, const int x, const int y)  {
	return false;
}
