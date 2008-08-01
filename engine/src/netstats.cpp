#include "netstats.h"
#include "config.h"
#include "mrt/logger.h"
#include <map>

NetStats::NetStats() : pings_idx(0), pings_n(0), ping(0), deltas_idx(0), deltas_n(0), delta(0) {
	GET_CONFIG_VALUE("multiplayer.pings-samples", int, ps, 10);
	GET_CONFIG_VALUE("multiplayer.deltas-samples", int, ds, 15);
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

typedef std::map<const NetStats::delta_t, unsigned> gist_t;

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

	gist_t gist;
		
	for(unsigned i = 0; i < deltas_n; ++i) {
		delta_t d = deltas[i] - delta;
		++gist[d];
	}
	
	delta_t max1 = 0, max2 = 0;
	unsigned max_value = 0;	
	for(gist_t::iterator i = gist.begin(); i != gist.end(); ++i) {
		//LOG_DEBUG(("%ld -> %u ", (long)i->first, i->second));
		if (max_value == 0 || i->second > max_value) {
			max1 = max2 = i->first;
			max_value = i->second;
		} else if (i->second == max_value) {
			max2 = i->first;
		}
	}
	//LOG_DEBUG(("max at %ld-%ld = %u", (long)max1, (long)max2, max_value));

	delta += (max1 + max2) / 2;
	
	//sanity check : 
	/*
	gist.clear();
	for(unsigned i = 0; i < deltas_n; ++i) {
		delta_t d = deltas[i] - delta;
		++gist[d];
	}
	
	for(gist_t::iterator i = gist.begin(); i != gist.end(); ++i) {
		LOG_DEBUG(("%ld -> %u ", (long)i->first, i->second));
	}
	*/
	return (public_delta_t)delta;
}
