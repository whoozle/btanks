#include "simple_gamepad_setup.h"
#include "resource_manager.h"
#include "box.h"
#include "label.h"
#include "i18n.h"
#include "window.h"
#include "menu.h"
#include "chooser.h"

SimpleGamepadSetup::SimpleGamepadSetup() : bg_table(ResourceManager->loadSurface("menu/gamepad_table.png")), selection(NULL) {
	int joys = joy.getCount();
	if (joys <= 0)
		throw_ex(("no gamepad found"));
	
	add(0, 0, bg = new Box("menu/background_box_dark.png", bg_table->get_width() + 96, bg_table->get_height() + 140, 24));
	int bw, bh, mx, my;
	bg->get_size(bw, bh);
	bg->getMargins(mx, my);
	
	bg_table_pos = v2<int>((bw - bg_table->get_width()) / 2, (bh - bg_table->get_height()) / 2);
	
	std::vector<std::string> jlist;
	for(int i = 0; i < joys; ++i) 
		jlist.push_back(joy.getName(i));
	joy_list = new Chooser("small", jlist);
	joy.open(0);

	int cw, ch;
	joy_list->get_size(cw, ch);
	add((bw - cw) / 2, my, joy_list);
	
	const char * labels[] = {"up", "down", "left", "right", "fire", "alt-fire", "disembark", "hint-ctrl"};
	size_t n = sizeof(labels) / sizeof(labels[0]);
	ch = (bg_table->get_height() - 46) / n;
	for(size_t i = 0; i < n; ++i) {
		add(bg_table_pos.x + 8, bg_table_pos.y + 47 + i * ch, new Label("medium", I18n->get("menu", labels[i])));
	}

	on_event_slot.assign(this, &SimpleGamepadSetup::on_event, Window->event_signal);
}

void SimpleGamepadSetup::on_event(const SDL_Event &event) {
	//LOG_DEBUG(("event!"));
	if (hidden() || active_row < 0 || active_row >= 8) 
		return;

	switch(event.type) {
		case SDL_JOYAXISMOTION: 
		case SDL_JOYHATMOTION: 
		case SDL_JOYBUTTONDOWN: 
			LOG_DEBUG(("wow!"));
			break;

		default:; 
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

void SimpleGamepadSetup::hide(const bool hide) {
	MainMenu::generate_key_events_for_gamepad = hide;
	Container::hide(hide);
}

void SimpleGamepadSetup::tick(const float dt) {
	if (joy_list->changed()) {
		joy_list->reset();
		joy.open(joy_list->get());
	}
}
