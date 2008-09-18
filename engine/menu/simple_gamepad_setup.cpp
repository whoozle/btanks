#include "simple_gamepad_setup.h"
#include "resource_manager.h"
#include "box.h"
#include "label.h"
#include "i18n.h"
#include "window.h"
#include "menu.h"
#include "chooser.h"
#include "math/unary.h"
#include "slider.h"
#include "config.h"
#include "button.h"

SimpleGamepadSetup::SimpleGamepadSetup() : bg_table(ResourceManager->loadSurface("menu/gamepad_table.png")), selection(NULL) {
	int joys = joy.getCount();
	if (joys <= 0)
		throw_ex(("no gamepad found"));
	
	add(0, 0, bg = new Box("menu/background_box_dark.png", bg_table->get_width() + 96, bg_table->get_height() + 140, 24));
	int bw, bh, mx, my;
	bg->get_size(bw, bh);
	bg->getMargins(mx, my);
	
	
	std::vector<std::string> jlist;
	for(int i = 0; i < joys; ++i) 
		jlist.push_back(joy.getName(i));
	joy_list = new Chooser("small", jlist);

	int cw, ch;
	joy_list->get_size(cw, ch);
	add((bw - cw) / 2, my, joy_list);

	bg_table_pos = v2<int>((bw - bg_table->get_width()) / 2, my + ch + my);
	
	const char * labels[] = {"left", "right", "up", "down", "fire", "alt-fire", "disembark", "hint-ctrl"};
	size_t n = sizeof(labels) / sizeof(labels[0]);
	ch = (bg_table->get_height() - 46) / n;
	for(size_t i = 0; i < n; ++i) {
		add(bg_table_pos.x + 8, bg_table_pos.y + 47 + i * ch, new Label("medium", I18n->get("menu", labels[i])));
		add(bg_table_pos.x + 160, bg_table_pos.y + 50 + i * ch, controls[i] = new Label("small", std::string()));
	}
	
	int ybase = bg_table_pos.y + bg_table->get_height() + my; 
	dead_zone = new Slider(1);

	init(0);
	dead_zone->get_size(cw, ch);
	add((bw - cw) / 2, ybase, dead_zone);
	ybase += ch;

	_b_ok = new Button("medium_dark", I18n->get("menu", "ok"));
	_b_ok->get_size(cw, ch);
	add(3 * bw / 4 - cw / 2, ybase, _b_ok);

	_b_default = new Button("medium_dark", I18n->get("menu", "revert-to-defaults"));
	_b_default->get_size(cw, ch);
	add(bw / 4 - cw / 2, ybase, _b_default);
	
	on_event_slot.assign(this, &SimpleGamepadSetup::on_event, Window->event_signal);
	_modal = true;
}

void SimpleGamepadSetup::init(const int idx) {
	joy.open(idx);
	profile = joy.getName(idx);
	joy_list->set(idx);
	bindings = SimpleJoyBindings(profile, joy);
	dead_zone->set(bindings.get_dead_zone());
	refresh();
}

void SimpleGamepadSetup::refresh() {
	for(unsigned i = 0; i < 8; ++i) {
		controls[i]->set(bindings.get_name(i));
	}
}

void SimpleGamepadSetup::on_event(const SDL_Event &event) {
	//LOG_DEBUG(("event!"));
	if (hidden() || active_row < 0 || active_row >= 8) 
		return;

	switch(event.type) {
		case SDL_JOYAXISMOTION: {
			const SDL_JoyAxisEvent &je = event.jaxis;
			int v = math::abs(je.value);
			if (v < (int)(32767 * dead_zone->get())) 
				break;
			//LOG_DEBUG(("axis %d: %d", je.axis, je.value));
		 	bindings.set(active_row, SimpleJoyBindings::State(SimpleJoyBindings::State::Axis, je.axis, je.value > 0? 1: -1));
		 	refresh();
		 	//++active_row;
			break;
		}
		case SDL_JOYHATMOTION: {
			const SDL_JoyHatEvent &je = event.jhat;
			if (je.value == SDL_HAT_CENTERED)
				break;
			
			//LOG_DEBUG(("hat %d: %04x", je.hat, (unsigned)je.value));
			bindings.set(active_row, SimpleJoyBindings::State(SimpleJoyBindings::State::Hat, je.hat, je.value));
		 	refresh();
		 	//++active_row;
			break;
		}

		case SDL_JOYBUTTONDOWN: {
			const SDL_JoyButtonEvent &je = event.jbutton;
			//LOG_DEBUG(("button %d", je.button));
			bindings.set(active_row, SimpleJoyBindings::State(SimpleJoyBindings::State::Button, je.button, 0));
		 	refresh();
			//++active_row;
			break;
		}

		default: 
			return;
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

void SimpleGamepadSetup::revert_to_default() {
	bindings.clear();
	refresh();
}

void SimpleGamepadSetup::save() {
	bindings.save();
}


bool SimpleGamepadSetup::onKey(const SDL_keysym sym) {
	if (Container::onKey(sym))
		return true;
	
	switch(sym.sym) {
	case SDLK_RETURN:
	case SDLK_ESCAPE:
		save();
		hide();
		return true;
	
	default:
		return true;
	}
}

bool SimpleGamepadSetup::onMouse(const int button, const bool pressed, const int x, const int y) {
	if (Container::onMouse(button, pressed, x, y))
		return true;
	//blah blah

	return true;
}

bool SimpleGamepadSetup::onMouseMotion(const int state, const int x, const int y, const int xrel, const int yrel) {
	if (Container::onMouseMotion(state, x, y, xrel, yrel))
		return true;
	
	active_row = y - bg_table_pos.y - 44;
	if (active_row < 0) 
		return true;

	active_row /= 30;
	return true;
}

void SimpleGamepadSetup::hide(const bool hide) {
	MainMenu::generate_key_events_for_gamepad = hide;
	Container::hide(hide);
}

void SimpleGamepadSetup::tick(const float dt) {
	if (joy_list->changed()) {
		init(joy_list->get());
		joy_list->reset();
	}
	if (dead_zone->changed()) {
		dead_zone->reset();
		bindings.set_dead_zone(dead_zone->get());
	}
}
