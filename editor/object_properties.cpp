#include "object_properties.h"
#include "menu/text_control.h"
#include "menu/label.h"
#include "menu/box.h"
#include "menu/popup_menu.h"
#include "object.h"

void ObjectPropertiesDialog::reset() {
	_z->set(0);
	Container::reset();
}

const int ObjectPropertiesDialog::get_z() const {
	return _z->get();
}

void ObjectPropertiesDialog::get(std::set<std::string> &labels) const {
	_menu->get(labels);
}

void ObjectPropertiesDialog::tick(const float dt) {
	Container::tick(dt);
	if (_menu->changed()) {
		_menu->reset();
		invalidate(false);
	}
}


bool ObjectPropertiesDialog::onKey(const SDL_keysym sym) {
	switch(sym.sym) {

	case SDLK_ESCAPE: 
		hide();
		return true;
		
	case SDLK_KP_ENTER:
	case SDLK_RETURN: 
		hide();
		invalidate();
		return true;

	default: 
		return Container::onKey(sym);
	}
}


void ObjectPropertiesDialog::show(const Object *o, const std::set<std::string> &variants) {
	assert(o != NULL);
	object = o;
	
	_menu->clear();
	for(std::set<std::string>::const_iterator i = variants.begin(); i != variants.end(); ++i) {
		_menu->append(*i, o->get_variants().has(*i));
	}
	_z->set(o->get_z());
	hide(false);
}

ObjectPropertiesDialog::ObjectPropertiesDialog(const int w) {
	int xp = 0, yp = 0;

	int sw, sh;
	Control *label = new Label("medium", "z: ");
	label->get_size(sw, sh);
	
	Box * box = new Box("menu/background_box.png", w, sh + 16);
	add(xp, yp, box);
	int mx, my; 
	box->getMargins(mx, my);
	
	int bx, by;
	box->get_size(bx, by);
	yp += by / 2 - sh / 2;
	xp += mx;
	
	add(xp, yp, label);
	
	_z = new NumericControl("medium", 0);
	add(xp + sw, yp, _z);
	_z->get_size(bx, by);
	yp += by + 3 * my;
	
	_menu = new PopupMenu();
	add(xp, yp, _menu);
}
