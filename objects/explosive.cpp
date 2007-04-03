#include "destructable_object.h"
#include "resource_manager.h"

class Explosive : public DestructableObject {
public: 
	Explosive();
	virtual void onBreak();

	Object* clone() const  {
		return new Explosive(*this);
	}
};

Explosive::Explosive() : DestructableObject("explosive-object", "fire", "fire", true) {}

void Explosive::onBreak() {
	spawn("cannon-explosion", "cannon-explosion");
}

REGISTER_OBJECT("explosive", Explosive, ());
