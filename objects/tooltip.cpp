#ifndef __OBJECTS_TOOLTIP_H__
#define __OBJECTS_TOOLTIP_H__

#include "resource_manager.h"
#include "object.h"
#include "mrt/random.h"
#include "sdlx/surface.h"

class Tooltip : public Object {
public:
	Tooltip() : Object("tooltip") { impassability = 0; }
	virtual void onSpawn() {
		const sdlx::Surface * s = getSurface();
		int w = s->getWidth();
		int n = (w - 1) / (int)size.x + 1;
		setDirectionsNumber(n);
		LOG_DEBUG(("dirs = %d", n));
		Object::setDirection(mrt::random(n));
		play("main", true);
	}
	
	virtual void setDirection(const int dir) {}

	virtual Object * clone() const {
		return new Tooltip(*this);
	}
};

REGISTER_OBJECT("random-tooltip", Tooltip, ());

#endif

