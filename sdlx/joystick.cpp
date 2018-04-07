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

#include "sdlx.h"

#include "sdlx/joystick.h"
#include "sdlx/sdl_ex.h"

using namespace sdlx;

int Joystick::getCount() {
	const int c = SDL_NumJoysticks();
	if (c < 0)
		throw_sdl(("SDL_NumJoysticks"));
	return c;
}

std::string Joystick::getName(const int idx) {
	const char * name_str = SDL_JoystickName(_joy);
	if (name_str == NULL)
		throw_sdl(("SDL_JoystickName(%d)", idx));
	std::string name = name_str;
	mrt::trim(name);
	return name;
}

void Joystick::sendEvents(const bool enable) {
	SDL_JoystickEventState(enable ? SDL_ENABLE: SDL_IGNORE);
}


Joystick::Joystick() : _joy(NULL) {}

void Joystick::open(const int idx) {
	close();
	_joy = SDL_JoystickOpen(idx);
	if (_joy == NULL)
		throw_sdl(("SDL_JoystickOpen(%d)", idx));
}

Joystick::Joystick(const int idx) : _joy (SDL_JoystickOpen(idx)) {
	if (_joy == NULL)
		throw_sdl(("SDL_JoystickOpen(%d)", idx));
}

const bool Joystick::opened() const {
	return _joy != NULL;
}

Sint16 Joystick::get_axis(const int idx) const {
	if (_joy == NULL)
		throw_ex(("get_axis(%d) on uninitialized joystick", idx));
	return SDL_JoystickGetAxis(_joy, idx);
}

const bool Joystick::get_button(const int idx) const {
	if (_joy == NULL)
		throw_ex(("get_button(%d) on uninitialized joystick", idx));
	return SDL_JoystickGetButton(_joy, idx) != 0;
}

const int Joystick::get_hat(const int idx) const {
	if (_joy == NULL)
		throw_ex(("get_hat(%d) on uninitialized joystick", idx));
	return SDL_JoystickGetHat(_joy, idx);
}

void Joystick::get_ball(const int idx, int &dx, int &dy) const {
	if (_joy == NULL)
		throw_ex(("get_ball(%d) on uninitialized joystick", idx));
	if (SDL_JoystickGetBall(_joy, idx, &dx, &dy) == -1)	
		throw_sdl(("SDL_JoystickGetBall(%d)", idx));
}


const int Joystick::get_axis_num() const {
	if (_joy == NULL)
		throw_ex(("get_axis_num() on uninitialized joystick"));
	return SDL_JoystickNumAxes(_joy);
}

const int Joystick::get_buttons_num() const {
	if (_joy == NULL)
		throw_ex(("get_buttons_num() on uninitialized joystick"));
	return SDL_JoystickNumButtons(_joy);
}

const int Joystick::get_balls_num() const {
	if (_joy == NULL)
		throw_ex(("get_balls_num() on uninitialized joystick"));
	return SDL_JoystickNumBalls(_joy);
}

const int Joystick::get_hats_num() const {
	if (_joy == NULL)
		throw_ex(("get_balls_num() on uninitialized joystick"));
	return SDL_JoystickNumHats(_joy);
}

void Joystick::close() {
	if (_joy == NULL) 
		return;
	
	SDL_JoystickClose(_joy);
	_joy = NULL;
}

Joystick::~Joystick() {
	close();
}
