#ifndef __BTANKS_PLAYER_STATE_H__
#define __BTANKS_PLAYER_STATE_H__

struct PlayerState {
	bool left, right, up, down, fire;
	PlayerState() { clear(); }
	void clear() { memset(this, 0, sizeof(*this)); }
};

#endif

