#include "simple_gamepad_setup.h"
#include "resource_manager.h"
#include "box.h"

SimpleGamepadSetup::SimpleGamepadSetup() : bg_table(ResourceManager->loadSurface("menu/gamepad_table.png")) {
	add(0, 0, bg = new Box("menu/background_box_dark.png", bg_table->get_width() + 96, bg_table->get_height() + 140));
	int bw, bh, mx, my;
	bg->get_size(bw, bh);
	bg->getMargins(mx, my);
	
	bg_table_pos = v2<int>((bw - bg_table->get_width()) / 2, (bh - bg_table->get_height()) / 2);
}

void SimpleGamepadSetup::render(sdlx::Surface &surface, const int x, const int y) const {
	Container::render(surface, x, y);
	surface.blit(*bg_table, x + bg_table_pos.x, y + bg_table_pos.y);
}

bool SimpleGamepadSetup::onKey(const SDL_keysym sym) {
	switch(sym.sym) {
	case SDLK_ESCAPE: 
	case SDLK_RETURN:
		hide();
		return true;
	
	default:
		return true;
	}
}

bool SimpleGamepadSetup::onMouse(const int button, const bool pressed, const int x, const int y) {
	return true;
}
