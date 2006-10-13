#include "player_slot.h"
#include "controls/control_method.h"

PlayerSlot::PlayerSlot() : id(-1), obj(NULL), control_method(NULL), need_sync(false), remote(false), trip_time(10), visible(false), 
mapx(0), mapy(0), mapvx(0), mapvy(0)
{}

PlayerSlot::PlayerSlot(Object *obj) : id(-1), obj(obj), control_method(NULL), need_sync(false), remote(false), trip_time(10), visible(false), 
mapx(0), mapy(0), mapvx(0), mapvy(0)
{}

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
