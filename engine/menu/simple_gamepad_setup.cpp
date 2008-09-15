#include "simple_gamepad_setup.h"
#include "resource_manager.h"
#include "box.h"
#include "label.h"
#include "i18n.h"

SimpleGamepadSetup::SimpleGamepadSetup() : bg_table(ResourceManager->loadSurface("menu/gamepad_table.png")), selection(NULL) {
	add(0, 0, bg = new Box("menu/background_box_dark.png", bg_table->get_width() + 96, bg_table->get_height() + 140, 24));
	int bw, bh, mx, my;
	bg->get_size(bw, bh);
	bg->getMargins(mx, my);
	
	bg_table_pos = v2<int>((bw - bg_table->get_width()) / 2, (bh - bg_table->get_height()) / 2);
	

	const char * labels[] = {"up", "down", "left", "right", "fire", "alt-fire", "disembark", "hint-ctrl"};
	size_t n = sizeof(labels) / sizeof(labels[0]);
	int ch = (bg_table->get_height() - 46) / n;
	for(size_t i = 0; i < n; ++i) {
		add(bg_table_pos.x + 8, bg_table_pos.y + 47 + i * ch, new Label("medium", I18n->get("menu", labels[i])));
	}
}

void SimpleGamepadSetup::render(sdlx::Surface &surface, const int x, const int y) const {
	AUTOLOAD_SURFACE(selection, "menu/gamepad_selection.png");
	
	Container::render(surface, x, y);
	surface.blit(*bg_table, x + bg_table_pos.x, y + bg_table_pos.y);
	if (active_row >= 0 && active_row < 8) {
		surface.blit(*selection, x + bg_table_pos.x + 152, y + bg_table_pos.y + 44 + 30 * active_row);
	}
}

bool SimpleGamepadSetup::onKey(const SDL_keysym sym) {
	switch(sym.sym) {
	case SDLK_ESCAPE: 
		reload();
		hide();
		return true;

	case SDLK_RETURN:
		save();
		hide();
		return true;
	
	default:
		return true;
	}
}

bool SimpleGamepadSetup::onMouse(const int button, const bool pressed, const int x, const int y) {
	return true;
}

bool SimpleGamepadSetup::onMouseMotion(const int state, const int x, const int y, const int xrel, const int yrel) {
	active_row = (y - bg_table_pos.y - 44) / 30;
	return true;
}
