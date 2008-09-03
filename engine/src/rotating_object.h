#ifndef BTANKS_ENGINE_ROTATING_OBJECT_H__
#define BTANKS_ENGINE_ROTATING_OBJECT_H__

#include "export_btanks.h"
#include "object.h"
namespace sdlx {
	class Surface;
}

class BTANKSAPI RotatingObject : public Object {
public: 
	float angle_speed;
	RotatingObject(const std::string &classname);
	RotatingObject(const RotatingObject &ro);
	
	virtual void calculate(const float dt);
	virtual void tick(const float dt);
	virtual void render(sdlx::Surface &surface, const int x, const int y);
	virtual ~RotatingObject();

private: 
	float angle, cached_angle;
	std::string last_state;
	int last_pos;
	sdlx::Surface *cached_surface, *src_surface;
};

#endif

