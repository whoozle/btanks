#include "object.h"
#include "context.h"
#include "locker.h"

using namespace clunk;

Object::Object(Context *context) : context(context) {}

void Object::update(const v3<float> &pos, const v3<float> &vel) {
	AudioLocker l;
	position = pos;
	velocity = vel;
}

void Object::play(Source *source) {
	AudioLocker l;
	sources.insert(source);
}

void Object::remove(Source *source) {
	AudioLocker l;
	sources.erase(source);
}

void Object::remove_all() {
	AudioLocker l;
	sources.clear();
}

Object::~Object() {
	AudioLocker l;
	context->delete_object(this);
}
