#ifndef BTANKS_NETSTATS_H__
#define BTANKS_NETSTATS_H__

#include <vector>
#include "export_btanks.h"

#ifndef _WINDOWS
#	include <stdint.h>
#endif

class BTANKSAPI NetStats {
public:
#ifdef _WINDOWS
	typedef __int64 delta_t;
	typedef __int32 public_delta_t;
#else
	typedef int64_t delta_t;
	typedef int32_t public_delta_t;
#endif

	NetStats();
	float updatePing(const float ping);
	public_delta_t updateDelta(const public_delta_t delta);

	inline float getPing() const { return ping;}
	inline public_delta_t getDelta() const { return (public_delta_t)delta; } 
	void clear();

private: 
	std::vector<float> pings;
	unsigned pings_idx, pings_n; 
	float ping;
	
	std::vector<int> deltas;
	unsigned deltas_idx, deltas_n;

	delta_t delta;
};

#endif

