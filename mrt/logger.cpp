/* M-runtime for c++
 * Copyright (C) 2005-2007 Vladimir Menshakov
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */
#include "logger.h"
#include <stdio.h>

#ifdef WIN32
#	define WINDOWS_LEAN_AND_MEAN
#	include <windows.h>
#else
#	include <string.h>
#	include <sys/time.h>
#	include <time.h>
#endif

using namespace mrt;

IMPLEMENT_SINGLETON(Logger, ILogger)

ILogger::ILogger() : _level(LL_DEBUG), _lines(0) {}

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
	++_lines;
	int h = 0, m = 0, s = 0, ms = 0;
#ifdef WIN32
	struct _SYSTEMTIME st;
	GetSystemTime(&st);
	h = st.wHour; m = st.wMinute; s = st.wSecond; ms = st.wMilliseconds;
#else
	struct timeval tv;
	memset(&tv, 0, sizeof(tv));
	gettimeofday(&tv, NULL);
	
	struct tm tm;
	localtime_r(&tv.tv_sec, &tm);
	
	h = tm.tm_hour;
	m = tm.tm_min;
	s = tm.tm_sec;
	ms = tv.tv_usec / 1000;
#endif
	fprintf(stderr, "[%02d:%02d:%02d.%03d][%s:%d]\t [%s] %s\n", h, m, s, ms, file, line, getLogLevelName(level), str.c_str());
}

