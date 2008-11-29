
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

#include "sdlx/font.h"
#include "resource_manager.h"
#include "i18n.h"
#include "menu_item.h"
#include "sound/mixer.h"

MenuItem::MenuItem(const std::string &font, const std::string &area, const std::string &msg) : 
	font(ResourceManager->loadFont(font, true)),
	id(msg)
	{
	text = I18n->get(area, msg);
}

void MenuItem::get_size(int& x, int& y) const {
	x = font->render(NULL, x, y, text);
	y = font->get_height();
}

bool MenuItem::onKey(const SDL_keysym sym) {
	switch(sym.sym) {
	case SDLK_RETURN:
	case SDLK_SPACE:
	case SDLK_KP_ENTER:
		invalidate();
		Mixer->playSample(NULL, "menu/select.ogg", false);
		return true;
	default: 
		return false;
	}
}

bool MenuItem::onMouse(const int button, const bool pressed, const int x, const int y) {
	if (!pressed) {
		invalidate();
		Mixer->playSample(NULL, "menu/select.ogg", false);
	} else {
		Mixer->playSample(NULL, "menu/change.ogg", false);
	}
	return true;
}

void MenuItem::render(sdlx::Surface& window, int x, int y) const {
	font->render(window, x, y, text);
}
