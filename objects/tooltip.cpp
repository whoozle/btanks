#include "resource_manager.h"
#include "object.h"
#include "config.h"
#include "mrt/random.h"
#include "sdlx/surface.h"

class Tooltip : public Object {
public:
	Tooltip() : Object("tooltip"), _change(true) { impassability = 0; }
	virtual void onSpawn() {
		GET_CONFIG_VALUE("objects.random-tooltip.show-time", float, st, 3.0);
		_change.set(st);
	
		const sdlx::Surface * s = getSurface();
		int w = s->getWidth();
		int n = (w - 1) / (int)size.x + 1;
		setDirectionsNumber(n);
		LOG_DEBUG(("dirs = %d", n));
		Object::setDirection(mrt::random(n));
		play("main", true);
	}
	
	virtual void tick(const float dt) {
		Object::tick(dt);
		if (_change.tick(dt)) {
			Object::setDirection(mrt::random(getDirectionsNumber()));
		}
	}
	
	virtual void setDirection(const int dir) {}

	virtual Object * clone() const {
		return new Tooltip(*this);
	}
private: 
	Alarm _change;
};

REGISTER_OBJECT("random-tooltip", Tooltip, ());
