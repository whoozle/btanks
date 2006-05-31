#include "fmt.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

using namespace mrt;

const std::string mrt::formatString(const char *fmt, ...) {
	char buf[1024];
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
