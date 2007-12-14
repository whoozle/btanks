#ifndef BTANKS_PROFILER_H__
#define BTANKS_PROFILER_H__

#include <string>
#include <map>
#include "sdlx/timer.h"

class Profiler {
public: 
	Profiler();
	
	void reset();
	void add(const std::string &object);
	
	void add(const std::string &object, const int time);
	~Profiler();
private: 
	typedef std::map<const std::string, int> Samples;
	Samples samples;
	sdlx::Timer timer;
};

#endif

