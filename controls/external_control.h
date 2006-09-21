#ifndef __BTANKS_EXTERNAL_CONTROL_METHOD_H__
#define __BTANKS_EXTERNAL_CONTROL_METHOD_H__


class ExternalControl : public ControlMethod {
public:
	PlayerState state;
	virtual void updateState(PlayerState &s) { s = state; }
};

#endif

