#include "sdlx/window.h"

namespace sdlx {
	Window::Window(const std::string &title, int w, int h, Uint32 flags): _window(SDL_CreateWindow(title.c_str(), 0, 0, w, h, flags)) {
	}

	Window::~Window() {
	}
}
