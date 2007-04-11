/* sdlx - c++ wrapper for libSDL
 * Copyright (C) 2005-2007 Vladimir Menshakov
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

#include "sdlx.h"

#include "sdlx/joystick.h"
#include "sdlx/sdl_ex.h"

using namespace sdlx;

const int Joystick::getCount() {
	const int c = SDL_NumJoysticks();
	if (c < 0)
		throw_sdl(("SDL_NumJoysticks"));
	return c;
}

const std::string Joystick::getName(const int idx) {
	const char * name = SDL_JoystickName(idx);
	if (name == NULL)
		throw_sdl(("SDL_JoystickName(%d)", idx));
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

Sint16 Joystick::getAxis(const int idx) const {
	if (_joy == NULL)
		throw_ex(("getAxis(%d) on uninitialized joystick", idx));
	return SDL_JoystickGetAxis(_joy, idx);
}

const bool Joystick::getButton(const int idx) const {
	if (_joy == NULL)
		throw_ex(("getButton(%d) on uninitialized joystick", idx));
	return SDL_JoystickGetButton(_joy, idx) != 0;
}

const int Joystick::getHat(const int idx) const {
	if (_joy == NULL)
		throw_ex(("getHat(%d) on uninitialized joystick", idx));
	return SDL_JoystickGetHat(_joy, idx);
}

void Joystick::getBall(const int idx, int &dx, int &dy) const {
	if (_joy == NULL)
		throw_ex(("getBall(%d) on uninitialized joystick", idx));
	if (SDL_JoystickGetBall(_joy, idx, &dx, &dy) == -1)	
		throw_sdl(("SDL_JoystickGetBall(%d)", idx));
}


const int Joystick::getNumAxes() const {
	if (_joy == NULL)
		throw_ex(("getNumAxes() on uninitialized joystick"));
	return SDL_JoystickNumAxes(_joy);
}

const int Joystick::getNumButtons() const {
	if (_joy == NULL)
		throw_ex(("getNumButtons() on uninitialized joystick"));
	return SDL_JoystickNumButtons(_joy);
}

const int Joystick::getNumBalls() const {
	if (_joy == NULL)
		throw_ex(("getNumBalls() on uninitialized joystick"));
	return SDL_JoystickNumBalls(_joy);
}

const int Joystick::getNumHats() const {
	if (_joy == NULL)
		throw_ex(("getNumBalls() on uninitialized joystick"));
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
