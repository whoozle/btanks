#include "netstats.h"
#include "config.h"
#include "mrt/logger.h"

NetStats::NetStats() : pings_idx(0), pings_n(0), ping(0), deltas_idx(0), deltas_n(0), delta(0) {
	GET_CONFIG_VALUE("multiplayer.pings-samples", int, ps, 10);
	GET_CONFIG_VALUE("multiplayer.deltas-samples", int, ds, 5);
	pings.resize(ps);
	deltas.resize(ds);
}

void NetStats::clear() {
	pings_idx = 0;
	pings_n = 0;
	ping = 0;
	deltas_idx = 0;
	deltas_n = 0;
	delta = 0;
}

float NetStats::updatePing(const float p) {
	size_t n = pings.size();
	if (pings_n < n)
		++pings_n;
	
	pings[pings_idx++] = p;
	pings_idx %= n;
	
	ping = 0;
	for(unsigned i = 0; i < pings_n; ++i) {
		ping += pings[i];
	}
	ping /= pings_n;
	return ping;
}

NetStats::public_delta_t NetStats::updateDelta(const int d) {
	//LOG_DEBUG(("updateDelta(%d)", d));
	size_t n = deltas.size();
	if (deltas_n < n)
		++deltas_n;
	
	deltas[deltas_idx++] = d;
	deltas_idx %= n;
	
	delta = 0;
	for(unsigned i = 0; i < deltas_n; ++i) {
		delta += deltas[i];
		//LOG_DEBUG(("+%d %d", deltas[i], delta));
	}

	delta /= (delta_t)deltas_n;
	//LOG_DEBUG(("delta: %d, reduced: %d", delta, math::reduce(delta, (int)ping)));
	return (public_delta_t)delta;
}
