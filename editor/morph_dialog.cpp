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

#include "morph_dialog.h"
#include "tmx/map.h"
#include "tmx/generator.h"

MorphDialog::MorphDialog(const int w, const int h) : ScrollList("menu/background_box_dark.png", "small", w, h) {
	init_map_slot.assign(this, &MorphDialog::initMap, Map->load_map_signal);
}

void MorphDialog::initMap() {
	LOG_DEBUG(("refresh boxes list"));
	clear();
	std::deque<std::pair<std::string, std::string> > boxes;
	Map->getGenerator()->getPrimaryBoxes(boxes);
	for(size_t i = 0; i < boxes.size(); ++i) {
		append(boxes[i].first + ", " + boxes[i].second);
	}
	reset();
}

bool MorphDialog::onKey(const SDL_keysym sym) {
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
