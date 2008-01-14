#include "resize_dialog.h"
#include "menu/number_control.h"
#include "tmx/map.h"
#include "menu/box.h"

ResizeDialog::ResizeDialog() : done(false) {
	c_l = new NumberControl("small", -99, 99);
	c_r = new NumberControl("small", -99, 99);
	c_u = new NumberControl("small", -99, 99);
	c_d = new NumberControl("small", -99, 99);

	int cw, ch, w, h;
	c_l->getSize(cw, ch);

	Box * box = new Box("menu/background_box.png", cw * 4, ch * 4);
	box->getSize(w, h);
	int x = (w - 3 * cw) / 2, y = (h - 3 * ch) / 2;

	add(0, 0, box);
	
	add(x, y + ch, c_l);
	add(x + cw, y, c_u);
	add(x + cw * 2, y + ch, c_r);
	add(x + cw, y + ch * 2, c_d);
}

void ResizeDialog::show() {
	const v2<int> map_size = Map->getSize() / Map->getTileSize();
	c_l->setMinMax(-map_size.x, 99);
	c_r->setMinMax(-map_size.x, 99);
	c_u->setMinMax(-map_size.y, 99);
	c_d->setMinMax(-map_size.y, 99);
	c_l->set(0);
	c_r->set(0);
	c_d->set(0);
	c_u->set(0);
	hide(false);
}

const bool ResizeDialog::get(int &left, int &right, int &up, int &down) const {
	if (!done)
		return false;
	done = false;

	left = c_l->get();
	right = c_r->get();
	up = c_u->get();
	down = c_d->get();
	return true;
}

void ResizeDialog::resize() {
	invalidate();
	TRY { 
		Map->resize(c_l->get(), c_r->get(), c_u->get(), c_d->get());
	} CATCH("resize", );
}

bool ResizeDialog::onKey(const SDL_keysym sym) {
	if (Container::onKey(sym))
		return true;
	switch(sym.sym) {
	
	case SDLK_RETURN: 
		hide();
		resize();
		return true;

	case SDLK_ESCAPE: 
		hide();
		return true;

	default: 
		return false;
	}
}
