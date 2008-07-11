#ifndef BTANKS_NETSTATS_H__
#define BTANKS_NETSTATS_H__

#include <vector>
#include "export_btanks.h"

#ifndef _WINDOWS
#	include <stdint.h>
#endif

class BTANKSAPI NetStats {
public:
	NetStats();
	float updatePing(const float ping);
	int updateDelta(const int delta);

	inline float getPing() const { return ping;}
	inline int getDelta() const { return delta; } 
	void clear();

private: 
	std::vector<float> pings;
	unsigned pings_idx, pings_n; 
	float ping;
	
	std::vector<int> deltas;
	unsigned deltas_idx, deltas_n;
#ifdef _WINDOWS
	__int64 delta;
#else
	int64_t delta;
#endif
};

#endif

