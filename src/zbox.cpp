#include "zbox.h"
#include "math/unary.h"
#include <assert.h>

const bool ZBox::sameBox(const int z1, const int z2) {
	return getBox(z1) == getBox(z2);	
}

const int ZBox::getBox(const int z) {
	return (z / 1000  + math::sign(z) ) / 2;
}

const int ZBox::getBoxBase(const int z) {
	return getBox(z) * 2000;
}
