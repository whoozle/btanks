#ifndef BTANKS_LAYER_LIST_DIALOG_H__
#define BTANKS_LAYER_LIST_DIALOG_H__

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

#include "menu/scroll_list.h"
#include "sl08/sl08.h"

class LayerItem;
class Prompt;

class LayerListDialog : public ScrollList {
public: 
	LayerListDialog(const int w, const int h);
	virtual bool onKey(const SDL_Keysym sym); 
	virtual void tick(const float dt);
	
	const bool active() const;
	
	const LayerItem * getCurrentItem() const;
	const LayerItem * getItem(const int idx) const;

private: 
	void initMap();
	sl08::slot0<void, LayerListDialog> init_map_slot;
	Prompt *_new_layer;
};

#endif

