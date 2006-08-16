#include "fmt.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

#if defined WIN32 && !defined vsnprintf
#define vsnprintf _vsnprintf
#endif

using namespace mrt;

const std::string mrt::formatString(const char *fmt, ...) {
	char buf[4096];
	memset(buf, 0, sizeof(buf));
	
	va_list ap;
	
    va_start(ap, fmt);
    vsnprintf (buf, sizeof(buf)-1, fmt, ap);
    va_end(ap);
	return buf;
}

void mrt::trim(std::string &str, const std::string chars) {
	size_t i = str.find_first_not_of(chars);
	if (i > 0)
		str.erase(0, i);
	
	i = str.find_last_not_of(chars);
	if (i != str.npos)
		str.erase(i + 1, str.size());
}

void mrt::split(std::vector<std::string> & result, const std::string &str, const std::string &delimiter, const int limit) {
	result.clear();
	
	std::string::size_type pos = 0, p;
	int n = limit;
	
	while(pos < str.size()) {
		if (n > 0) {
			if (n-- == 0) 
				return;
		}
		p = str.find(delimiter, pos);
		if (p != std::string::npos) 
			result.push_back(str.substr(pos, p - pos));
		else {
			result.push_back(str.substr(pos));
			return;
		}
		pos = p + delimiter.size();
	}
}
