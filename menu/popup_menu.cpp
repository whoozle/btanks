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
	}
	const bool getState() const { return state; }
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
		const ToggleLabel * l = dynamic_cast<const ToggleLabel *>(i->second);
		if (l == NULL) 
			continue;
		if (l->getState())
			labels.insert(l->get());
	}		
}

void PopupMenu::append(const std::string &item, const bool state) {
	int w, h;
	getSize(w, h);
	add(0, h + 5, new ToggleLabel(item, state));
	getSize(w, h);
	w += 32; h += 24;
	_background->init("menu/background_box_dark.png", "menu/highlight_medium.png", w, h);
}

bool PopupMenu::onMouse(const int button, const bool pressed, const int x, const int y) {
	if (Container::onMouse(button, pressed, x, y))
		return true;
	if (pressed)
		return true;
	
	for(ControlList::iterator i = _controls.begin(); i != _controls.end(); ++i) {
		ToggleLabel * l = dynamic_cast<ToggleLabel *>(i->second);
		if (l == NULL) 
			continue;

		int bw, bh;
		l->getSize(bw, bh);
	
		const sdlx::Rect dst(i->first.x, i->first.y, bw, bh);
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
		const ToggleLabel * l = dynamic_cast<const ToggleLabel *>(i->second);
		if (l == NULL) 
			continue;

		int bw, bh;
		l->getSize(bw, bh);
	
		const sdlx::Rect dst(i->first.x, i->first.y, bw, bh);
		if (dst.in(x, y)) {
			hl_pos = i->first;
			hl_pos.x -= 16;
			hl_pos.y += 9;
		}
	}
	
	return false;
}

void PopupMenu::render(sdlx::Surface &surface, const int x, const int y) {
	int mx, my;
	_background->getMargins(mx, my);

	_background->render(surface, x - mx, y - my);
	Container::render(surface, x, y);
	
	if (hl_pos.x == -1 || hl_pos.y == -1) 
		return;
	
	_background->renderHL(surface, x + hl_pos.x, y + hl_pos.y);
}
