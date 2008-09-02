#ifndef BTANKS_ENGINE_ROTATING_OBJECT_H__
#define BTANKS_ENGINE_ROTATING_OBJECT_H__

#include "export_btanks.h"
#include "sdlx/surface.h"
#include "object.h"

class BTANKSAPI RotatingObject : public Object {
public: 
	float angle_speed;
	RotatingObject(const std::string &classname);
	void calculate(const float dt);

private: 
	float angle;
	sdlx::Surface cached_surface;
};

#endif

