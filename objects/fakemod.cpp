#include "fakemod.h"
#include "registrar.h"

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

FakeMod::FakeMod() : Object("fake-mod"), _type(), _n(0) {
	hp = -1;
	impassability = 0;
	pierceable = true;
}

Object * FakeMod::clone() const {
	return new FakeMod(*this);
}

void FakeMod::setCount(const int n) {
	_n = n;
}

void FakeMod::decreaseCount(const int n) {
	_n -= n;
	if (_n < 0) 
		_n = 0;
}

const std::string FakeMod::getType() const {
	return _type;
}
const int FakeMod::getCount() const {
	return _n;
}

void FakeMod::setType(const std::string &type) {
	_type = type;
}


void FakeMod::on_spawn() {
	play("main", true);
}

void FakeMod::render(sdlx::Surface &surface, const int x, const int y) {}
void FakeMod::tick(const float dt) {}
void FakeMod::calculate(const float dt) {}


void FakeMod::serialize(mrt::Serializator &s) const {
	Object::serialize(s);
	s.add(_type);
	s.add(_n);
}

void FakeMod::deserialize(const mrt::Serializator &s) {
	Object::deserialize(s);
	s.get(_type);
	s.get(_n);
}

REGISTER_OBJECT("fake-mod", FakeMod, ());
