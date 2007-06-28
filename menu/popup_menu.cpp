#include "popup_menu.h"
#include "box.h"
#include "label.h"

PopupMenu::PopupMenu() {
	_background = new Box(); 
	//add(0, 0, _background);
}

void PopupMenu::append(const std::string &item) {
	int w, h;
	getSize(w, h);
	add(0, h, new Label("medium_dark", item));
	getSize(w, h);
	_background->init("menu/background_box_dark.png", "menu/highlight_medium.png", w, h);
}

bool PopupMenu::onMouse(const int button, const bool pressed, const int x, const int y) {
	if (Container::onMouse(button, pressed, x, y))
		return true;
		
	for(ControlList::const_iterator i = _controls.begin(); i != _controls.end(); ++i) {
		const Label * l = dynamic_cast<const Label *>(i->second);
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
	return Container::onMouseMotion(state, x, y, xrel, yrel);
}
