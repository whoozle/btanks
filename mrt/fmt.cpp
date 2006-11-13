#include "fmt.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

#if defined WIN32 
#	if !defined vsnprintf
#		define vsnprintf _vsnprintf
#	endif

#	include <malloc.h>
#	if !defined alloca
#		define alloca _alloca
#	endif

#endif

using namespace mrt;

#define FORMAT_BUFFER_SIZE 4096

//#include "logger.h"

const std::string mrt::formatString(const char *fmt, ...) {
	int size = FORMAT_BUFFER_SIZE;
	char *buf;
	va_list ap;

    while(1) {
    	buf = (char *)alloca(size);
	    va_start(ap, fmt);    
    	int r = vsnprintf (buf, size - 1, fmt, ap);
	    va_end(ap);
	    if (r > -1 && r <= size) 
    		return std::string(buf, r);
    	size *= 2;
    }
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
