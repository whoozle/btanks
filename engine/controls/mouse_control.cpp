
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

#include "mouse_control.h"
#include "mrt/logger.h"
#include "player_manager.h"
#include "window.h"
#include "player_slot.h"
#include "object.h"
#include "math/unary.h"
#include "object.h"
#include "sdlx/cursor.h"
#include "tmx/map.h"

MouseControl::MouseControl(): _shoot(false), _shoot_alt(false), bleave(false), alt_fire(0.3f, false) {
	on_mouse_slot.assign(this, &MouseControl::onMouse, Window->mouse_signal);
} 

bool MouseControl::onMouse(const int button, const bool pressed, const int x, const int y) {
	//LOG_DEBUG(("button %d,%d %d: %c", x, y, button, pressed?'+':'-'));
	if (button == SDL_BUTTON_RIGHT) { //fixme: hardcoded
		_shoot = pressed;
		return true;
	}
	if (button == SDL_BUTTON_MIDDLE) {
		bleave = pressed;
		return true;
	}
#warning port me (mouse wheels)
#if 0
	if (button == SDL_BUTTON_WHEELUP || button == SDL_BUTTON_WHEELDOWN) { //fixme: hardcoded
		_shoot_alt = true;
		alt_fire.reset();
		return true;
	}
#endif
	if (!pressed) {
		if (button == SDL_BUTTON_LEFT) 
			target_screen_set = false;
		return true;
	}
	
	//LOG_DEBUG(("shoot: %c, move: %c", _shoot?'+':'-', _move?'+':'-'));
/*	v2<float> world;
	PlayerManager->screen2world(world, 0, x, y); //fixme!! hardcoded player number
	if (_shoot) {
		Object *o = getObject();
		if (o->get_target_position(_target, world, "bullet"))
			_target += o->get_position();
	}
	else _target = world;
	
	int dir = (world - pos).get_direction8() - 1;
	if (dir) {
		_target_dir = dir - 1;
		LOG_DEBUG(("target_dir = %d", _target_dir));
		assert(_target_dir >= 0);
	}
*/
	target_screen.x = x;
	target_screen.y = y;
	target_screen_set = true;
	
	return true;
}

void MouseControl::_updateState(PlayerSlot &slot, PlayerState &state, const float dt) {
	if (!sdlx::Cursor::enabled())
		sdlx::Cursor::Enable();

	if (target_screen.is0()) {
		return;
	}

	Object *object = slot.getObject();
	if (object == NULL)
		return;
	
	v2<float> pos;
	object->get_center_position(pos);

	if (target_screen_set) {
		//hack hack hack
#warning track latest event
		//sdlx::Cursor::get_position(target_screen.x, target_screen.y);
		//target_screen_set = false;
		_target = slot.screen2world(target_screen);

		_target_rel = Map->distance(pos, _target);

		_target_dir = object->get_direction();
	}
	
	{
		v2<float> velocity = Map->distance(pos, _target);
		int dirs = object->get_directions_number();
		switch(dirs) {
			case 8: 
				velocity.quantize8(); break;
			case 16:
				velocity.quantize16(); break;
		}

		if (velocity.x * _target_rel.x <= 0) {
			_target_rel.x = 0;
		}
		if (velocity.y * _target_rel.y <= 0) {
			_target_rel.y = 0;
		}
	}
	
	//state.fire = _target_rel.is0() && _shoot;
	//if (state.fire) {
	//	object->set_direction(_target_dir);
	//}
	
	if (_target_rel.x == 0) {
		state.left = state.right = false;
	} else {
		state.left = _target_rel.x < 0;
		state.right = !state.left;
	}


	if (_target_rel.y == 0) {
		state.up = state.down = false;
	} else {
		state.up = _target_rel.y < 0;
		state.down = !state.up;
	}
	
	bool trigger = alt_fire.tick(dt);
	if (trigger) {
		_shoot_alt = false;
	}
	
	state.fire = _shoot?1:0;
	state.alt_fire = _shoot_alt && !trigger;
	state.leave = bleave?1:0;
}

void MouseControl::get_name(std::vector<std::string> &controls, const PlayerState &state) const {
	if (state.left || state.right || state.up || state.down)
		controls.push_back(get_button_name(0)); //lmb
	if (state.fire) 
		controls.push_back(get_button_name(1)); //rmb
	if (state.alt_fire) 
		controls.push_back(get_button_name(3)); //4 wheel up/down
	if (state.leave) 
		controls.push_back(get_button_name(2)); //mmb
}

const std::string MouseControl::get_button_name(int idx) {
	if (idx < 0 || idx > 5)
		return mrt::format_string("(mouse %d)", idx);
	//11 100 010    10 010 001
	std::string r = "\342\221";
	r += (char)(0xaa + idx);
	return r;
}
