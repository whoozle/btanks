#ifndef __BTANKS_OBJECT_COMMON_H__
#define __BTANKS_OBJECT_COMMON_H__

#include "math/v3.h"
#include <deque>

typedef v3<int> WayPoint;
typedef std::deque<WayPoint> Way;

enum GroupType { Fixed, Centered };

#endif

