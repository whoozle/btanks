#include "netstats.h"
#include "config.h"
#include "math/binary.h"

NetStats::NetStats() : pings_idx(0), pings_n(0), ping(0), deltas_idx(0), deltas_n(0), delta(0) {
	GET_CONFIG_VALUE("multiplayer.statistical-samples", int, ss, 5);
	pings.resize(ss);
	deltas.resize(ss);
}

float NetStats::updatePing(const float p) {
	size_t n = pings.size();
	if (pings_n < n)
		++pings_n;
	
	pings[(pings_idx++) % n] = p;
	ping = 0;
	for(unsigned i = 0; i < pings_n; ++i) {
		ping += pings[i];
	}
	ping /= pings_n;
	return ping;
}

int NetStats::updateDelta(const int d) {
	size_t n = deltas.size();
	if (deltas_n < n)
		++deltas_n;
	
	deltas[(deltas_idx++) % n] = d;
	delta = 0;
	for(unsigned i = 0; i < pings_n; ++i) {
		delta += deltas[i];
	}
	delta /= deltas_n;
	delta -= (int)ping;
	return delta;
}
