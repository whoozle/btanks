#ifndef __BTANKS_EXTERNAL_CONTROL_METHOD_H__
#define __BTANKS_EXTERNAL_CONTROL_METHOD_H__

#include "player_state.h"
#include "control_method.h"

class ExternalControl : public ControlMethod {
public:
	PlayerState state;
	virtual void updateState(PlayerState &s);
};

#endif

