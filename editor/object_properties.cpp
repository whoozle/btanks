/* Battle Tanks Game
 * Copyright (C) 2006-2008 Battle Tanks team
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
