#include "object.h"
#include "context.h"

using namespace clunk;

Object::Object(Context *context) : context(context) {}

void Object::updatePV(const v3<float> &pos, const v3<float> &vel) {
	//blah blah 
}

Object::~Object() {
	context->deleteObject(this);
}
