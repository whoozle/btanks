#include "zbox.h"
#include "math/unary.h"
#include <assert.h>

ZBox::ZBox(const v3<int> &position, const v2<int> &size) : position(position), size(size) {}

const bool ZBox::operator<(const ZBox &other) const {
	if (position != other.position)
		return position < other.position;
	if (size != other.size)
		return size < other.size;
	return false;
}

const bool ZBox::in(const v3<int> &p) const {
	if (getBox(position.z) != getBox(p.z))
		return false;
	return (
		p.x >= position.x && p.y >= position.y && 
		p.x < position.x + size.x && p.y < position.y + size.y
	);
}


const bool ZBox::sameBox(const int z1, const int z2) {
	return getBox(z1) == getBox(z2);	
}

const int ZBox::getBox(const int z) {
	return (z / 1000  + math::sign(z) ) / 2;
}

const int ZBox::getBoxBase(const int z) {
	return getBox(z) * 2000;
}
