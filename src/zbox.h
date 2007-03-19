#ifndef BTANKS_ZBOX_H__
#define BTANKS_ZBOX_H__

#include "math/v2.h"
#include "math/v3.h"

class ZBox {
public: 
	v3<int> position;
	v2<int> size;

	ZBox(const v3<int> &position, const v2<int> &size);
	const bool operator<(const ZBox &other) const;
	const bool in(const v3<int> &position) const;

	static const bool sameBox(const int z1, const int z2);
	static const int getBox(const int z);
	static const int getBoxBase(const int z);
};

#endif
