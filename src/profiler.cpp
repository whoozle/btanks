#include "profiler.h"
#include "mrt/logger.h"
#include "utils.h"

Profiler::Profiler() {}

void Profiler::add(const std::string &object, const int t, const float dt) {
	struct data & d = samples[object];
	if (t > 0)
		d.micros += t;
	if (t > d.peak)
		d.peak = t;
	if (dt > 0)
		d.life_time += dt;
}

void Profiler::create(const std::string &object) {
	++samples[object].objects;
}

void Profiler::reset() {
	timer.reset();
}

void Profiler::add(const std::string &object, const float dt) {
	int t = timer.microdelta();
	add(object, t, dt);
}

void Profiler::dump() {
	if (samples.empty())
		return;
	
	LOG_NOTICE(("[object name]                    mcS      peak     count    lifetime avg.load"));
	typedef std::multimap<const double, std::pair<std::string, data> , std::greater<double> > Results;
	Results results;
	
	for(Samples::const_iterator i = samples.begin(); i != samples.end(); ++i) {
		const data & d = i->second;
		double avg = d.micros / d.life_time;
		results.insert(Results::value_type(avg, std::pair<std::string, data>(i->first, d)));
	}
	
	for(Results::const_iterator i = results.begin(); i != results.end(); ++i) {
		const data & d = i->second.second;
		LOG_NOTICE(("%-32s %-8d %-8d %-8d %-8g %-8g", i->second.first.c_str(), d.micros, d.peak, d.objects, d.life_time, d.micros / d.life_time));
	}

	samples.clear();
}

Profiler::~Profiler() {}
