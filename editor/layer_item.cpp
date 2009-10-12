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

#include "layer_item.h"
#include "mrt/logger.h"
#include "tmx/layer.h"
#include "menu/label.h"
#include "menu/checkbox.h"
#include <assert.h>

LayerItem::LayerItem(const int z, Layer *layer) : z(z), layer(layer) {
	assert(layer != NULL);
	
	int cw, ch;
	const std::string name = mrt::format_string("%3d: %s", z, layer->name.c_str());
	int xp = 0;
	
	_c_show = new Checkbox(layer->visible);
	_c_show->get_size(cw, ch);
	add(xp, 0, _c_show);
	xp += cw + 4;

	_c_solo = new Checkbox();
	_c_show->get_size(cw, ch);
	add(xp, 0, _c_solo);
	xp += cw + 4;
	
	int max_h = ch;
	
	Label *l = new Label("small", name);
	l->get_size(cw, ch);
	add(xp, (max_h - ch) / 2, l);
	xp += cw + 4;
}

bool LayerItem::onMouse(const int button, const bool pressed, const int x, const int y) {
	bool r = Container::onMouse(button, pressed, x, y);
	if (r && in(_c_solo, x, y)) {
		return false;
	}
	return r;
}


void LayerItem::tick(const float dt) {
	Container::tick(dt);
	if (_c_show->changed()) {
		_c_show->reset();
		layer->visible = _c_show->get();
		//LOG_DEBUG(("layer %s %s", _layer->name.c_str(), _layer->visible?"visible":"invisible"));
	}
	if (_c_solo->changed()) {
		_c_solo->reset();
		layer->solo = _c_solo->get();	
	}
}
