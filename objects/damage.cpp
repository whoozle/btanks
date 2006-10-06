#include "object.h"
#include "mrt/logger.h"
#include "resource_manager.h"

class Damage : public Object {
public:
	Damage();
	virtual void render(sdlx::Surface &surface, const int x, const int y);
	virtual void onSpawn();
	//virtual void tick(const float dt);
	virtual Object * clone() const;
};


Damage::Damage() : Object("damage-digits") { impassability = 0; }
Object * Damage::clone() const { return new Damage(*this); } 

void Damage::onSpawn() { 
	play("main", true); 
	_state.up = true; 
	setZ(999); 
} 
/*
void Damage::tick(const float dt) {
		
}
*/

void Damage::render(sdlx::Surface &surface, const int x, const int y) {
	int digits = 1;
	int mult = 1;
	int n;
	
	for(n = hp; n >= 10; n/=10) {
		++digits;
		mult *= 10;
	}
	//LOG_DEBUG(("number: %d, digits = %d, mult: %d", hp, digits, mult));
	int xp = x;
	n = hp;
	while(digits--) {
		int d = n / mult;
		
		n %= mult;
		mult /= 10;
		//LOG_DEBUG(("digit %d", d));
		setDirection(d);
		Object::render(surface, xp, y);
		xp += (int)size.x;
	}
}

REGISTER_OBJECT("damage-digits", Damage, ());
