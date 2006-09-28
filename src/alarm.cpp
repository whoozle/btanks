#include "alarm.h"

Alarm::Alarm(const float period, const bool repeat) : _period(period), _t(period), _repeat(repeat) {}

const bool Alarm::tick(const float dt) {
	if (_t == 0)
		return true;
	int n = (int) (dt / _period);
	
	_t -= (dt - (n * _period));
	
	if (_t <= 0) {
		_t = _repeat?_period + _t:0;
		return true;
	}
	return false;
}

void Alarm::reset() {
	_t = _period;
}
