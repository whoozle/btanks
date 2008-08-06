
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
#include "control.h"
#include "sound/mixer.h"

Control::Control() : _changed(false), _mouse_in(false), _hidden(false) {}


void Control::tick(const float dt) {}

void Control::activate(const bool active) {
}

void Control::hide(const bool hide) {
	_hidden = hide;
}

bool Control::onKey(const SDL_keysym sym) {
	return false;
}

bool Control::onMouse(const int button, const bool pressed, const int x, const int y) {
	return false;
}

bool Control::onMouseMotion(const int state, const int x, const int y, const int xrel, const int yrel) {
	return false;
}

void Control::on_mouse_enter(bool enter) {
	//LOG_DEBUG(("%s", enter?"enter":"leave"));
}

void Control::invalidate(const bool play_sound) {
	if (play_sound && !_changed)
		Mixer->playSample(NULL, "menu/change.ogg", false);
	_changed = true;
}

Control::~Control() {}
