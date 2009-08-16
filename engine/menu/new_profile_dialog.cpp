#include "new_profile_dialog.h"
#include "text_control.h"
#include "button.h"
#include "i18n.h"
#include "label.h"
#include "box.h"

NewProfileDialog::NewProfileDialog() {
	Box * box = new Box("menu/background_box_dark.png", 32, 32);
	add(-16, -8, box);
	
	Label *l = new Label("medium", I18n->get("menu", "enter-profile-name"));
	int w, h, cw, ch;
	l->get_size(w, ch);
	add(0, 8, l);

	int yp = ch + 16;
	
	_name = new TextControl("small", 32);
	_name->get_size(cw, ch);
	add((w - 192) / 2, yp, _name);
	yp += ch + 8;

	_ok = new Button("medium_dark", I18n->get("menu", "ok"));
	_ok->get_size(cw, ch);
	add((w - cw) / 2, yp, _ok);
	
	get_size(w, h);
	w += 32;
	h += 16;

	box->init("menu/background_box_dark.png", w, h);
}

bool NewProfileDialog::onKey(const SDL_keysym sym) {
	if (Container::onKey(sym))
		return true;
		
	if (sym.sym == SDLK_RETURN || sym.sym == SDLK_KP_ENTER) {
		_name->invalidate(true);
		return true;
	} 
	return false;
}

void NewProfileDialog::tick(const float dt) {
	Container::tick(dt);
	if (_name->changed() || _ok->changed()) {
		_name->reset();
		_ok->reset();
		if (!_name->get().empty())
			invalidate(true);
	}
}
