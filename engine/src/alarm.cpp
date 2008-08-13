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

#include "alarm.h"
#include "mrt/serializator.h"
#include <assert.h>
#include <math.h>

Alarm::Alarm(const float period, const bool repeat) : _period(period), _t(0), _repeat(repeat) {}
Alarm::Alarm(const bool repeat): _period(0), _t(0), _repeat(repeat) {}

const bool Alarm::tick(const float dt) {
	assert(_period > 0);
	if (dt < 0) {
		return false;
	}
	
	if (!_repeat) {
		if (_t < _period)
			_t += dt;
		return _t >= _period;
	} else {
		_t += dt;
		if (_t < _period)
			return false;
			
		int n = (int)floor(_t / _period);
		_t -= _period * n;

		while(_t > _period && _t > 0) //paranoid stuff
			_t -= _period;
	
		return true;	
	}
}

const float Alarm::get() const {
	return (_t >= _period)?1: _t / _period;
}

void Alarm::set(const float period, const bool reset) {
	_period = period;
	if (reset)
		_t = 0;
}

void Alarm::reset() {
	_t = 0;
}

void Alarm::serialize(mrt::Serializator &s) const {
	s.add(_period);
	s.add(_t);
	s.add(_repeat);
}

void Alarm::deserialize(const mrt::Serializator &s) {
	s.get(_period);
	s.get(_t);
	s.get(_repeat);
}
