#include "object.h"
#include "resource_manager.h"

class SinglePose : public Object {
public:
	SinglePose(const std::string &pose, const bool repeat, const bool no_directions = false, const bool play_start = false) : 
		Object("single-pose"), _pose(pose), _repeat(repeat), _no_dir(no_directions), _play_start(play_start) {}

	virtual Object * clone() const;
	virtual void emit(const std::string &event, BaseObject * emitter = NULL);
	virtual void tick(const float dt);
	virtual void onSpawn();
	virtual void render(sdlx::Surface &surface, const int x, const int y);

private:
	std::string _pose;
	bool _repeat, _no_dir, _play_start;
};

void SinglePose::emit(const std::string &event, BaseObject * emitter) {
	if (event == "collision") {
		addDamage(emitter);
	} else Object::emit(event, emitter);
}

void SinglePose::render(sdlx::Surface &surface, const int x, const int y) {
	if (_no_dir)
		setDirection(0);
	Object::render(surface, x, y);
}


void SinglePose::tick(const float dt) {
	Object::tick(dt);
	if (getState().empty()) {	
		//LOG_DEBUG(("over"));
		emit("death", this);
	}
}

void SinglePose::onSpawn() {
	//LOG_DEBUG(("single-pose: play('%s', %s)", _pose.c_str(), _repeat?"true":"false"));
	play(_pose, _repeat);
	if (_play_start) {
		playNow("start");
	}
}


Object* SinglePose::clone() const  {
	Object *a = NULL;
	TRY {
		a = new SinglePose(*this);
	} CATCH("clone", { delete a; throw; });
	return a;
}

REGISTER_OBJECT("single-pose", SinglePose, ("main", true));
REGISTER_OBJECT("single-pose-with-start", SinglePose, ("main", true, false, true));
REGISTER_OBJECT("single-pose-once", SinglePose, ("main", false));
REGISTER_OBJECT("single-pose-no-directions", SinglePose, ("main", true, true));
REGISTER_OBJECT("single-pose-once-no-directions", SinglePose, ("main", false, true));
REGISTER_OBJECT("missile-launch", SinglePose, ("launch", false));
