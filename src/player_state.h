#ifndef __BTANKS_PLAYER_STATE_H__
#define __BTANKS_PLAYER_STATE_H__

class PlayerState {
public:
	bool left, right, up, down, fire, alt_fire;
	PlayerState() { clear(); }
	void clear() { memset(this, 0, sizeof(*this)); }
};

#endif

