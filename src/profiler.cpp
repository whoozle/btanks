#include "profiler.h"
#include "mrt/logger.h"
#include "utils.h"

Profiler::Profiler() {}

void Profiler::add(const std::string &object, const int t) {
	if (t > 0)
		samples[object].first += t;
}

void Profiler::create(const std::string &object) {
	++samples[object].second;
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
	typedef std::multimap<const int, ternary<std::string, int, int> , std::greater<int> > Results;
	Results results;
	
	for(Samples::const_iterator i = samples.begin(); i != samples.end(); ++i) {
		int avg = i->second.first / i->second.second;
		results.insert(Results::value_type(avg, ternary<std::string, int, int>(i->first, i->second.first, i->second.second)));
	}
	for(Results::const_iterator i = results.begin(); i != results.end(); ++i) {
		LOG_NOTICE(("%-32s\t%-8d %-8d %-8d", i->second.first.c_str(), i->second.second, i->second.third, i->second.second / i->second.third));
	}
}
