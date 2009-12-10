/* Battle Tanks Game
 * Copyright (C) 2006-2009 Battle Tanks team
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

/* 
 * Additional rights can be granted beyond the GNU General Public License 
 * on the terms provided in the Exception. If you modify this file, 
 * you may extend this exception to your version of the file, 
 * but you are not obligated to do so. If you do not wish to provide this
 * exception without modification, you must delete this exception statement
 * from your version and license this file solely under the GPL without exception. 
*/

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
	c_l->get_size(cw, ch);

	Box * box = new Box("menu/background_box.png", cw * 4, ch * 4);
	box->get_size(w, h);
	int x = (w - 3 * cw) / 2, y = (h - 3 * ch) / 2;

	add(0, 0, box);
	
	add(x, y + ch, c_l);
	add(x + cw, y, c_u);
	add(x + cw * 2, y + ch, c_r);
	add(x + cw, y + ch * 2, c_d);
}

void ResizeDialog::show() {
	const v2<int> map_size = Map->get_size() / Map->getTileSize();
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
	} CATCH("resize", {});
}

bool ResizeDialog::onKey(const SDL_keysym sym) {
	if (Container::onKey(sym))
		return true;
	switch(sym.sym) {
	
	case SDLK_KP_ENTER:
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
