#ifndef __BTANKS_ALARM_H__
#define __BTANKS_ALARM_H__

#include "mrt/serializable.h"

class Alarm : public mrt::Serializable {
public:
	Alarm(const float period, const bool repeat);
	Alarm(const bool repeat);
	const bool tick(const float dt);
	void reset();
	void set(const float period, const bool reset = true);

	virtual void serialize(mrt::Serializator &s) const;
	virtual void deserialize(const mrt::Serializator &s);

private: 
	float _period, _t;
	bool _repeat;
};

#endif

