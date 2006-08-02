#include "logger.h"
#include <stdio.h>

using namespace mrt;

IMPLEMENT_SINGLETON(Logger, ILogger)

ILogger::ILogger():_level(LL_DEBUG) {}

ILogger::~ILogger() {}

void ILogger::setLogLevel(const int level) {
	_level = level;
}

const char * ILogger::getLogLevelName(const int level) {
	switch(level) {
		case LL_DEBUG: return "debug";
		case LL_NOTICE: return "notice";
		case LL_WARN: return "warn";
		case LL_ERROR: return "error";
		default: return "unknown";
	}
}


void ILogger::log(const int level, const char *file, const int line, const std::string &str) {
	if (level < _level) return;
	fprintf(stderr, "[%s:%d]\t [%s] %s\n", file, line, getLogLevelName(level), str.c_str());
}

