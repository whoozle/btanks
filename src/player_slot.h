#ifndef __BTANKS_PLAYER_SLOT_H__
#define __BTANKS_PLAYER_SLOT_H__

#include "player_state.h"
#include "math/v3.h"
#include "sdlx/rect.h"
#include <string>

class Object;
class ControlMethod;

class PlayerSlot {
public:
	PlayerSlot();
	PlayerSlot(Object *obj);
	Object * obj;
	ControlMethod * control_method;
	v3<int> position;
		
	PlayerState state;
	bool need_sync;
	bool remote;
	float trip_time;
	
	bool visible;
	sdlx::Rect viewport;
		
	float mapx, mapy, mapvx, mapvy;
		
	void clear();
	~PlayerSlot();
		
	//respawn stuff.
	std::string classname;
	std::string animation;
};

#endif
