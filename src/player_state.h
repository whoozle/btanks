#ifndef __BTANKS_PLAYER_STATE_H__
#define __BTANKS_PLAYER_STATE_H__

#include "mrt/serializable.h"
#include <string.h>

class PlayerState {
public:
	bool left, right, up, down, fire, alt_fire;
	PlayerState() { clear(); }
	void clear() { memset(this, 0, sizeof(*this)); }
	
	inline const bool operator==(const PlayerState &other) const {
		return left == other.left && right == other.right && up == other.up && down == other.down &&
			fire == other.fire && alt_fire == other.fire;
	}
	inline const bool operator!=(const PlayerState &other) const {
		return !(*this == other);
	}

	virtual void serialize(mrt::Serializator &s) const;
	virtual void deserialize(const mrt::Serializator &s);
	virtual ~PlayerState() {}
};

#endif

