#include "control.h"

Control::Control() : _changed(false), _hidden(false) {}


void Control::tick(const float dt) {}

bool Control::onKey(const SDL_keysym sym) {
	return false;
}

bool Control::onMouse(const int button, const bool pressed, const int x, const int y) {
	return false;
}

bool Control::onMouseMotion(const int state, const int x, const int y, const int xrel, const int yrel) {
	return false;
}
