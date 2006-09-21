#include "object.h"
#include "resource_manager.h"

class SinglePose : public Object {
public:
	SinglePose(const std::string &pose, const bool repeat, const bool no_directions = false, const bool play_start = false, const bool breakable = false) : 
		Object("single-pose"), _pose(pose), _repeat(repeat), _no_dir(no_directions), _play_start(play_start), _breakable(breakable), _broken(false) {}

	virtual Object * clone() const;
	virtual void emit(const std::string &event, BaseObject * emitter = NULL);
	virtual void tick(const float dt);
	virtual void onSpawn();
	virtual void render(sdlx::Surface &surface, const int x, const int y);
	virtual void addDamage(BaseObject *from, const bool emitDeath = true);

	virtual void serialize(mrt::Serializator &s) const {
		Object::serialize(s);
		s.add(_pose);
		s.add(_repeat);
		s.add(_no_dir);
		s.add(_play_start);
		s.add(_breakable);
		s.add(_broken);
	}

	virtual void deserialize(const mrt::Serializator &s) {
		Object::deserialize(s);
		s.get(_pose);
		s.get(_repeat);
		s.get(_no_dir);
		s.get(_play_start);
		s.get(_breakable);
		s.get(_broken);
	}

private:
	std::string _pose;
	bool _repeat, _no_dir, _play_start, _breakable, _broken;
};

void SinglePose::addDamage(BaseObject *from, const bool emitDeath) {
	if (!_breakable) {
		BaseObject::addDamage(from, emitDeath);
		return;
	}

	if (_broken)
		return;

	BaseObject::addDamage(from, false);
	if (hp <= 0) {
		_broken = true;
		cancelAll();
		play("fade-out", false); 
		play("broken", true);
	}
}

void SinglePose::emit(const std::string &event, BaseObject * emitter) {
	Object::emit(event, emitter);
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

REGISTER_OBJECT("destructable-object", SinglePose, ("main", true, false, false, true));

REGISTER_OBJECT("missile-launch", SinglePose, ("launch", false));
