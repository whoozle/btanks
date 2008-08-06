#include "popup_menu.h"
#include "box.h"
#include "label.h"

class ToggleLabel : public Label {
public: 
	ToggleLabel(const std::string &item, const bool state) : Label("medium", item), state(state) {
		update();
	}
	void toggle() {
		state = !state;
		update();
	}
	const bool get_state() const { return state; }
private: 
	void update() {
		setFont(state?"medium_dark":"medium");
	}
	bool state;
};

PopupMenu::PopupMenu() : _background(new Box), hl_pos(-1, -1) {}
PopupMenu::~PopupMenu() { delete _background; }


void PopupMenu::clear() {
	Container::clear();
	hl_pos = v2<int>(-1, -1);
}

void PopupMenu::get(std::set<std::string> &labels) const {
	labels.clear();
	for(ControlList::const_iterator i = _controls.begin(); i != _controls.end(); ++i) {
		const ToggleLabel * l = dynamic_cast<const ToggleLabel *>(*i);
		if (l == NULL) 
			continue;
		if (l->get_state())
			labels.insert(l->get());
	}		
}

void PopupMenu::append(const std::string &item, const bool state) {
	int w, h;
	get_size(w, h);
	add(0, h + 5, new ToggleLabel(item, state));
	get_size(w, h);
	w += 32; h += 24;
	_background->init("menu/background_box_dark.png", w, h, 24);
}

bool PopupMenu::onMouse(const int button, const bool pressed, const int x, const int y) {
	if (Container::onMouse(button, pressed, x, y))
		return true;
	if (pressed)
		return true;
	
	for(ControlList::iterator i = _controls.begin(); i != _controls.end(); ++i) {
		ToggleLabel * l = dynamic_cast<ToggleLabel *>(*i);
		if (l == NULL) 
			continue;

		int bw, bh;
		l->get_size(bw, bh);
		int base_x, base_y;
		(*i)->get_base(base_x, base_y);
		const sdlx::Rect dst(base_x, base_y, bw, bh);
		if (dst.in(x, y)) {
			l->toggle();
			result = l->get();
			invalidate();
			return true;
		}
	}
	return true;
}

bool PopupMenu::onMouseMotion(const int state, const int x, const int y, const int xrel, const int yrel) {
	if (Container::onMouseMotion(state, x, y, xrel, yrel))
		return true;

	hl_pos = v2<int>(-1, -1);
	for(ControlList::const_iterator i = _controls.begin(); i != _controls.end(); ++i) {
		const ToggleLabel * l = dynamic_cast<const ToggleLabel *>(*i);
		if (l == NULL) 
			continue;

		int bw, bh;
		l->get_size(bw, bh);

		int base_x, base_y;
		(*i)->get_base(base_x, base_y);
		const sdlx::Rect dst(base_x, base_y, bw, bh);
		if (dst.in(x, y)) {
			hl_pos.x = base_x - 16;
			hl_pos.y = base_y + 9;
		}
	}
	
	return false;
}

void PopupMenu::render(sdlx::Surface &surface, const int x, const int y) const {
	if (_controls.empty())
		return;
	
	int mx, my;
	_background->getMargins(mx, my);

	_background->render(surface, x - mx, y - my);
	Container::render(surface, x, y);
	
	if (hl_pos.x == -1 || hl_pos.y == -1) 
		return;
	
	_background->renderHL(surface, x + hl_pos.x, y + hl_pos.y);
}
