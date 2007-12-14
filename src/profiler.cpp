#include "profiler.h"
#include "mrt/logger.h"

Profiler::Profiler() {}

void Profiler::add(const std::string &object, const int t) {
	if (t > 0)
		samples[object] += t;
}

void Profiler::reset() {
	timer.reset();
}

void Profiler::add(const std::string &object) {
	int t = timer.microdelta();
	add(object, t);
}

Profiler::~Profiler() {
	if (samples.empty())
		return;
	
	LOG_NOTICE(("profile results:"));
	for(Samples::const_iterator i = samples.begin(); i != samples.end(); ++i) {
		LOG_NOTICE(("\t%-20s\t%d", i->first.c_str(), i->second));
	}
}
