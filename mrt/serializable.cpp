#include "serializable.h"
#include "serializator.h"
#include "chunk.h"

using namespace mrt;

void Serializable::serialize2(mrt::Chunk &d) const {
	mrt::Serializator s;
	serialize(s);
	d = s.getData();
}

void Serializable::deserialize2(const mrt::Chunk &d) {
	mrt::Serializator s(&d);
	deserialize(s);
}
