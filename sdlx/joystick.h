#ifndef __SDLX_JOYSTICK_H__
#define __SDLX_JOYSTICK_H__

/* sdlx - c++ wrapper for libSDL
 * Copyright (C) 2005-2007 Vladimir Menshakov
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.

 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
*/

#include <string>
#include "sdlx.h"

namespace sdlx {
class SDLXAPI Joystick {
public: 
	static const int getCount();
	static const std::string getName(const int idx);
	static void sendEvents(const bool enable);
	
	Joystick();
	Joystick(const int idx);
	const bool opened() const;
	void open(const int idx);

	Sint16 get_axis(const int idx) const;
	const bool get_button(const int idx) const;
	const int get_hat(const int idx) const;
	void get_ball(const int idx, int &dx, int &dy) const;

	const int get_axis_num() const;
	const int get_buttons_num() const;
	const int get_balls_num() const;
	const int get_hats_num() const;

	void close();
	~Joystick();
private:
	Joystick(const Joystick &);
	const Joystick& operator=(const Joystick &);
	SDL_Joystick *_joy;
};
}


#endif

