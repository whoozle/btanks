
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

#include "player_state.h"
#include "mrt/serializator.h"
#include <string.h>
#include "mrt/fmt.h"

PlayerState::PlayerState() : left(false), right(false), up(false), down(false), fire(false), alt_fire(false), leave(false), hint_control(false) { } 
void PlayerState::clear() { left = right = up = down = fire = alt_fire = leave = hint_control = false; }

void PlayerState::serialize(mrt::Serializator &s) const {
	int packed = (left?1:0) | (right?2:0) | (up ? 4:0) | (down ? 8:0) | (fire ? 16:0) | (alt_fire ? 32:0) | (leave ? 64:0) | (hint_control ? 128:0);
	s.add(packed);
}

const bool PlayerState::operator==(const PlayerState &other) const {
	return left == other.left && right == other.right && up == other.up && down == other.down &&
		fire == other.fire && alt_fire == other.alt_fire && leave == other.leave && hint_control == other.hint_control;
}

bool PlayerState::compare_directions(const PlayerState &other) const {
	return left == other.left && right == other.right && up == other.up && down == other.down;
}

#define TEST_BIT(var, n) ((var & (1<<n)) != 0)

void PlayerState::deserialize(const mrt::Serializator &s) {
	int packed;
	s.get(packed);

	left = TEST_BIT(packed, 0);
	right = TEST_BIT(packed, 1);
	up = TEST_BIT(packed, 2);
	down = TEST_BIT(packed, 3);
	fire = TEST_BIT(packed, 4);
	alt_fire = TEST_BIT(packed, 5);
	leave = TEST_BIT(packed, 6);
	hint_control = TEST_BIT(packed, 7);
}

#define B(b) ((b)?'+':'-')

const std::string PlayerState::dump() const {
	return mrt::format_string("{ %c%c%c%c %c%c %c %c}", 
		B(left), B(right), B(up), B(down), B(fire), B(alt_fire), B(leave), B(hint_control));
}
