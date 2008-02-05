#ifndef BTANKS_PROFILER_H__
#define BTANKS_PROFILER_H__

#include <string>
#include <map>
#include "sdlx/timer.h"

class Profiler {
public: 
	Profiler();
	
	void reset();
	void add(const std::string &object, const float dt);
	void create(const std::string &object);
	
	~Profiler();
	void dump();
private: 
	void add(const std::string &object, const int t, const float dt);
	struct data {
		int micros;
		int objects;
		double life_time;
		int peak;
		data() : micros(0), objects(0), life_time(0), peak(0) {}
	};
	typedef std::map<const std::string, data> Samples;
	Samples samples;
	sdlx::Timer timer;
};

#endif

