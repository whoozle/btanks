#ifndef BTANKS_AI_RUSH_H__
#define BTANKS_AI_RUSH_H__

#include "export_btanks.h"
#include "object_common.h"

namespace ai {
class BTANKSAPI Rush {
public: 
	static void calculateW(Way &way, Object *object); //non-const Object* because of emitting death :)
};
}

#endif

