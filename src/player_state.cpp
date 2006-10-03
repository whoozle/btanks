#include "player_state.h"
#include "mrt/serializator.h"
#include <string.h>
#include "mrt/fmt.h"

PlayerState::PlayerState() { clear(); } 
void PlayerState::clear() { left = right = up = down = fire = alt_fire = false; }

void PlayerState::serialize(mrt::Serializator &s) const {
	int packed = (left?1:0) | (right?2:0) | (up ? 4:0) | (down ? 8:0) | (fire ? 16:0) | (alt_fire ? 32:0);
	s.add(packed);
}

void PlayerState::deserialize(const mrt::Serializator &s) {
	int packed;
	s.get(packed);
	left = packed & 1;
	right = packed & 2;
	up = packed & 4;
	down = packed & 8;
	fire = packed & 16;
	alt_fire = packed & 32;
}

#define B(b) ((b)?'+':'-')

const std::string PlayerState::dump() const {
	return mrt::formatString("{ %c%c%c%c%c%c }", 
		B(left), B(right), B(up), B(down), B(fire), B(alt_fire));
}
