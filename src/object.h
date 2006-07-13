#ifndef __WORLD_OBJECT_H__
#define __WORLD_OBJECT_H__

class Object {
public:
	float mass;
	Object();
	virtual ~Object();
	
	virtual void tick(const float dt) = 0;
protected:
	void setV(const float vx, const float vy);
private:
	float _vx, _vy;
	friend class IWorld;
};

#endif

