#ifndef __BTANKS_MOUSE_CONTROL_H__
#define __BTANKS_MOUSE_CONTROL_H__

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


#include "sdlx/joystick.h"
#include "control_method.h"
#include "player_state.h"
#include "math/v2.h"
#include "sl08/sl08.h"
#include "alarm.h"

class Object;
class MouseControl : public ControlMethod {
public:
	MouseControl(); 

private:
	virtual void _updateState(PlayerSlot &slot, PlayerState &state, const float dt);
	virtual void get_name(std::vector<std::string> &controls, const PlayerState &state) const;
	static const std::string get_button_name(int idx);
	
	void get_position(v2<float>&pos) const;
	Object * getObject() const;

	sl08::slot4<bool, const int, const bool, const int, const int, MouseControl> on_mouse_slot;	
	bool onMouse(const int button, const bool pressed, const int x, const int y);
	v2<int> target_screen;
	bool target_screen_set;

	v2<float> _target_rel, _target;
	int _target_dir;
	bool _shoot, _shoot_alt, bleave;
	Alarm alt_fire;
};

#endif
