#include "object.h"
#include "resource_manager.h"

class SinglePose : public Object {
public:
	SinglePose(const std::string &pose, const bool repeat) : Object("single-pose"), _pose(pose), _repeat(repeat) {}

	virtual Object * clone() const;
	virtual void emit(const std::string &event, BaseObject * emitter = NULL);
	virtual void tick(const float dt);
	virtual void onSpawn();
private:
	std::string _pose;
	bool _repeat;
};

void SinglePose::emit(const std::string &event, BaseObject * emitter) {
	if (event == "collision") {
		if (emitter->classname == "bullet") {
			hp -= emitter->hp;
			if (hp <= 0) {
				emit("death", emitter);
				spawn("explosion", "explosion", v3<float>(0,0,1), v3<float>());
			}
		}
	} else Object::emit(event, emitter);
}

void SinglePose::tick(const float dt) {
	Object::tick(dt);
	if (!getState().size()) {	
		//LOG_DEBUG(("over"));
		emit("death", this);
	}
}

void SinglePose::onSpawn() {
	//LOG_DEBUG(("single-pose: play('%s', %s)", _pose.c_str(), _repeat?"true":"false"));
	play(_pose, _repeat);
}


Object* SinglePose::clone() const  {
	Object *a = NULL;
	TRY {
		a = new SinglePose(*this);
	} CATCH("clone", { delete a; throw; });
	return a;
}

REGISTER_OBJECT("single-pose", SinglePose, ("main", true));
REGISTER_OBJECT("single-pose-once", SinglePose, ("main", false));
REGISTER_OBJECT("rocket-launch", SinglePose, ("launch", false));
