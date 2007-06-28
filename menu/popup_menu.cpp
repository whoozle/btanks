#include "popup_menu.h"
#include "box.h"
#include "label.h"

PopupMenu::PopupMenu() {
	//_background = new Box(); //("menu/background_box_dark.png", "menu/highlight_medium.png", w, h);
	//add(0, 0, _background);
}

void PopupMenu::append(const std::string &item) {
	int w, h;
	getSize(w, h);
	add(0, h, new Label("medium_dark", item));
}

bool PopupMenu::onKey(const SDL_keysym sym) {
	return Container::onKey(sym);
}

bool PopupMenu::onMouse(const int button, const bool pressed, const int x, const int y) {
	return Container::onMouse(button, pressed, x, y);
}

bool PopupMenu::onMouseMotion(const int state, const int x, const int y, const int xrel, const int yrel) {
	return Container::onMouseMotion(state, x, y, xrel, yrel);
}
