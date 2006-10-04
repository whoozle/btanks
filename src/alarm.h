#ifndef __BTANKS_ALARM_H__
#define __BTANKS_ALARM_H__


class Alarm {
public:
	Alarm(const float period, const bool repeat);
	Alarm(const bool repeat);
	const bool tick(const float dt);
	void reset();
	void set(const float period, const bool reset = true);

private: 
	float _period, _t;
	bool _repeat;
};

#endif

