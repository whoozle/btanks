#include "object.h"
#include "context.h"

using namespace clunk;

Object::Object(Context *context) : context(context) {}

void Object::update(const v3<float> &pos, const v3<float> &vel) {
	position = pos;
	velocity = vel;
}

void Object::play(Source *source) {
	sources.insert(source);
}

void Object::remove(Source *source) {
	sources.erase(source);
}

void Object::remove_all() {
	sources.clear();
}

Object::~Object() {
	context->delete_object(this);
}
