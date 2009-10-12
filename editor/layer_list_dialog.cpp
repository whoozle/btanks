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

#include "layer_list_dialog.h"
#include "layer_item.h"
#include "tmx/tileset_list.h"
#include "tmx/map.h"
#include "tmx/layer.h"
#include "menu/prompt.h"
#include "menu/text_control.h"

LayerListDialog::LayerListDialog(const int w, const int h) : ScrollList("menu/background_box.png", "small", w / 3, h, 0) {
	init_map_slot.assign(this, &LayerListDialog::initMap, Map->load_map_final_signal);

	_new_layer = new Prompt(320, 100, new TextControl("small"));
	int sw, sh;
	_new_layer->get_size(sw, sh);
	add(w / 3 - sw, (h - sh) / 2, _new_layer);
	_new_layer->hide();
}

void LayerListDialog::initMap() {
	clear();
	std::set<int> layers; 
	Map->getLayers(layers);
	for(std::set<int>::reverse_iterator i = layers.rbegin(); i != layers.rend(); ++i) {
		const int z = *i;
		Layer * layer = Map->getLayer(z);
		append(new LayerItem(z, layer));
	}
}

const LayerItem * LayerListDialog::getItem(const int idx) const {
	if (idx < 0 || idx >= (int)_list.size())
		throw_ex(("invalid index %d", idx));
	const LayerItem *li = dynamic_cast<const LayerItem *>(_list[idx]);
	if (li == NULL)
		throw_ex(("cast to LayerListItem failed"));
	return li;
}

const LayerItem * LayerListDialog::getCurrentItem() const {
	return getItem(_current_item);
}

const bool LayerListDialog::active() const { 
	return !_new_layer->hidden(); 
}

bool LayerListDialog::onKey(const SDL_keysym sym) {
	if (Container::onKey(sym))
		return true;
	
	switch (sym.sym) {
/*
	case SDLK_DELETE: {
			if (empty())
				return true;
			
			const int z = getCurrentItem()->z, idx = get();
			Map->deleteLayer(z);
			initMap();
			if (idx >= 0 && idx < (int)_list.size())
				set(idx);
		}
		return true;
*/
	case SDLK_TAB: 
		hide();
		return true;

	case SDLK_n: 
		LOG_DEBUG(("showing new layer dialog..."));		
		_new_layer->hide(false);
		return true;
	case SDLK_UP:
	case SDLK_DOWN: 
		if (sym.mod & KMOD_SHIFT) {
			const int z1 = getCurrentItem()->z;
			int idx = _current_item + ((sym.sym == SDLK_UP)? -1 : 1 );
			if (idx < 0 || idx >= (int)_list.size())
				return true;
			
			const int z2 = getItem(idx)->z;
			bool r = Map->swapLayers(z1, z2);
			if (r) {
				initMap();
				set(idx);
			}
			return true;
		}
	case SDLK_DELETE: 
		if ((sym.mod & KMOD_SHIFT) == 0)
			break;
		Map->deleteLayer(getItem(get())->z);
		return true;
	default: ;
	}
	ScrollList::onKey(sym);
	return true;
}

void LayerListDialog::tick(const float dt) {
	ScrollList::tick(dt);
	if (_new_layer->changed()) {
		LOG_DEBUG(("adding new layer..."));
		_new_layer->reset();
		_new_layer->hide();
		if (_new_layer->get().empty())
			return;

		std::string name = _new_layer->get();
		
		if (empty()) {
			//first layer: 
			LOG_DEBUG(("first layer..."));
			Map->addLayer(-1000, name);
			_new_layer->set(std::string());
			initMap();
			if (!empty())
				set(0);
			return;			
		}
		
		int current_z = get();
		bool ok = true;
		TRY {
			Map->addLayer(getCurrentItem()->z, name);
		} CATCH("tick", ok = false);
		if (ok) {
			initMap();
			_new_layer->set(std::string());
			set(current_z);
		} else {
			_new_layer->hide(false);
		}
	}
}
