#include "destructable_object.h"
#include "resource_manager.h"

class Explosive : public DestructableObject {
public: 
	
	Explosive() : DestructableObject("explosive-object", "fire", "fire", true) {}

	virtual void onBreak() {
		spawn("cannon-explosion", "cannon-explosion");
	}
};

REGISTER_OBJECT("explosive", Explosive, ());
