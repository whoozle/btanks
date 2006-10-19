#include "player_slot.h"
#include "world.h"
#include "controls/control_method.h"

PlayerSlot::PlayerSlot() : 
id(-1), control_method(NULL), need_sync(false), remote(false), trip_time(10), visible(false), 
mapx(0), mapy(0), mapvx(0), mapvy(0)
{}

PlayerSlot::PlayerSlot(const int id) : 
id(id), control_method(NULL), need_sync(false), remote(false), trip_time(10), visible(false), 
mapx(0), mapy(0), mapvx(0), mapvy(0)
{}

Object * PlayerSlot::getObject() {
	if (id < 0) 
		return NULL;
	Object *o = World->getObjectByID(id);
	return o;
}
const Object * PlayerSlot::getObject() const {
	if (id < 0) 
		return NULL;
	const Object *o = World->getObjectByID(id);
	return o;
}


void PlayerSlot::clear() {
	id = -1;
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
