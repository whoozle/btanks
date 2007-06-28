#include "popup_menu.h"
#include "box.h"
#include "label.h"

class ToggleLabel : public Label {
public: 
	ToggleLabel(const std::string &item, const bool state) : Label("medium", item), state(state) {
		update();
	}
	void setState(const bool state) {
		this->state = state;
		update();
	}
	const bool getState() const { return state; }
private: 
	void update() {
		setFont(state?"medim_dark":"medium");
	}
	bool state;
};

PopupMenu::PopupMenu() : _background(NULL) {
}

void PopupMenu::clear() {
	Container::clear();
	_background = NULL;
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
	add(0, h, new ToggleLabel(item, state));
	getSize(w, h);
	if (_background == NULL) {
		add(0, 0, _background = new Box);
	}
	_background->init("menu/background_box_dark.png", "menu/highlight_medium.png", w, h);
	int mx, my;
	_background->getMargins(mx, my);
	setBase(_background, -mx, -my);
}

bool PopupMenu::onMouse(const int button, const bool pressed, const int x, const int y) {
	if (Container::onMouse(button, pressed, x, y))
		return true;
		
	for(ControlList::const_iterator i = _controls.begin(); i != _controls.end(); ++i) {
		const ToggleLabel * l = dynamic_cast<const ToggleLabel *>(i->second);
		if (l == NULL) 
			continue;

		int bw, bh;
		l->getSize(bw, bh);
	
		const sdlx::Rect dst(i->first.x, i->first.y, bw, bh);
		if (dst.in(x, y)) {		
			result = l->get();
			invalidate();
			return true;
		}
	}
	return false;
}

bool PopupMenu::onMouseMotion(const int state, const int x, const int y, const int xrel, const int yrel) {
	if (Container::onMouseMotion(state, x, y, xrel, yrel))
		return true;
	
	return false;
}
