#ifndef __BTANKS_CONTROL_METHOD_H__
#define __BTANKS_CONTROL_METHOD_H__

class PlayerState;

class ControlMethod {
public:
	virtual void updateState(PlayerState &state) = 0;
	virtual ~ControlMethod() {}
};

#endif

