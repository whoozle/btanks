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
#include "fmt.h"
#include "chunk.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <ctype.h>
#include <algorithm>
#include "exception.h"

#if defined WIN32 
#	if !defined vsnprintf
#		define vsnprintf _vsnprintf
#	endif
/*
#	include <malloc.h>
#	if !defined alloca
#		define alloca _alloca
#	endif
*/

#endif

using namespace mrt;

#define FORMAT_BUFFER_SIZE 1024

//#include "logger.h"

const std::string mrt::formatString(const char *fmt, ...) {
	va_list ap;

	//try static buffer on the stack to avoid malloc calls
	char static_buf[FORMAT_BUFFER_SIZE];

    va_start(ap, fmt);    
   	int r = vsnprintf (static_buf, FORMAT_BUFFER_SIZE, fmt, ap);
    va_end(ap);

    if (r > -1 && r < FORMAT_BUFFER_SIZE) 
   		return std::string(static_buf, r);

	int size = FORMAT_BUFFER_SIZE * 2;

	mrt::Chunk buf;

    while(true) {
		buf.setSize(size);
	    va_start(ap, fmt);    
    	int r = vsnprintf ((char *)buf.getPtr(), size, fmt, ap);
	    va_end(ap);
	    if (r > -1 && r < size) 
    		return std::string((char *)buf.getPtr(), r);
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

void mrt::join(std::string &result, const std::vector<std::string>& array, const std::string &delimiter, const size_t limit) {
	result.clear();
	if (array.empty())
		return;
	
	size_t n = array.size();
	if (limit > 0 && limit < n) 
		n = limit;
	--n;
	for(size_t i = 0; i < n; ++i) {
		result += array[i];
		result += delimiter;
	}
	result += array[n];
}


void mrt::split(std::vector<std::string> & result, const std::string &str, const std::string &delimiter, const size_t limit) {
	result.clear();
	
	std::string::size_type pos = 0, p;
	size_t n = limit;
	
	while(pos < str.size()) {
		do {
			p = str.find(delimiter, pos);
			if (p == pos) {
				++p;
				++pos;
			}
		} while(p < str.size() && p == pos);
		
		
		if (p != std::string::npos) 
			result.push_back(str.substr(pos, p - pos));
		else {
			result.push_back(str.substr(pos));
			break;
		}
		if (n > 0) {
			if (--n == 0) {
				result[result.size() - 1] += str.substr(p);
				break;
			}
		}
		pos = p + delimiter.size();
	}
	if (limit)
		result.resize(limit);
}

void mrt::toUpper(std::string &str) {
	std::transform(str.begin(), str.end(), str.begin(), toupper);
}

void mrt::toLower(std::string &str) {
	std::transform(str.begin(), str.end(), str.begin(), tolower);
}

void mrt::replace(std::string &str, const std::string &from, const std::string &to, const size_t limit) {
	if (str.empty())
		return;
	
	if (from.empty())
		throw_ex(("replace string must not be empty"));
	
	std::string::size_type pos = 0;
	size_t n = limit;

	while(pos < str.size()) {
		pos = str.find(from, pos);
		if (pos == str.npos) 
			break;
	
		str.replace(pos, from.size(), to);
		pos += from.size() - to.size() + 1;
	
		if (n > 0) {
			if (--n == 0) {
				break;
			}
		}
	}	
}
