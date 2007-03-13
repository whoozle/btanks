#include "object.h"
#include "resource_manager.h"

class Teleport : public Object {
public: 
	Teleport() : Object("teleport") {}

	virtual void onSpawn();
	virtual Object * clone() const;
	
};

void Teleport::onSpawn() {
	play("main", true);
}

Object * Teleport::clone() const {
	return new Teleport(*this);
}

REGISTER_OBJECT("teleport", Teleport, ());
REGISTER_OBJECT("teleport-a", Teleport, ());

