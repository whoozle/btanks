#include "object.h"
#include "context.h"

using namespace clunk;

Object::Object(Context *context) : context(context) {}

void Object::updatePV(const v3<float> &pos, const v3<float> &vel) {
	this->pos = pos;
	this->vel = vel;
}

void Object::add(Source *source) {
	sources.push_back(source);
}

Object::~Object() {
	context->delete_object(this);
}
