#ifndef BTANKS_NETSTATS_H__
#define BTANKS_NETSTATS_H__

#include <vector>
#include "export_btanks.h"

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
	int delta;
};

#endif

