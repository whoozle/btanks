#include "player_slot.h"
#include "controls/control_method.h"

PlayerSlot::PlayerSlot() : obj(NULL), control_method(NULL), need_sync(false), remote(false), trip_time(10), visible(false) {}
PlayerSlot::PlayerSlot(Object *obj) : obj(obj), control_method(NULL), need_sync(false), remote(false), trip_time(10), visible(false) {}

void PlayerSlot::clear() {
	obj = NULL;
	if (control_method != NULL) {
		delete control_method; 
		control_method = NULL;
	}
	animation.clear();
	classname.clear();
	remote = false;
}

PlayerSlot::~PlayerSlot() {
	clear();
}
