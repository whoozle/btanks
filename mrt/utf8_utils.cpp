/* M-runtime for c++
 * Copyright (C) 2005-2008 Vladimir Menshakov
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.

 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
*/

#ifdef CURRENCY_SYMBOL
#	undef CURRENCY_SYMBOL
#endif

#include "tclUniData.c"

#include "utf8_utils.h"


#define WCHAR_T_SIZE 4

static const unsigned char totalBytes[256] = {
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
    2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,
    3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,
#if WCHAR_T_SIZE > 3
    4,4,4,4,4,4,4,4,
#else
    1,1,1,1,1,1,1,1,
#endif
#if WCHAR_T_SIZE > 4
    5,5,5,5,
#else
    1,1,1,1,
#endif
#if WCHAR_T_SIZE > 5
    6,6,6,6
#else
    1,1,1,1
#endif
};


unsigned mrt::wchar2lower(unsigned ch) {
    int info = GetUniCharInfo(ch);

    if (GetCaseType(info) & 0x02) {
		return (wchar_t) (ch + GetDelta(info));
    } else {
		return ch;
    }
}

unsigned mrt::wchar2upper(unsigned ch) {
	int info = GetUniCharInfo(ch);
	if (GetCaseType(info) & 0x04) {
		return (wchar_t) (ch - GetDelta(info));
	} else {
		return ch;
	}
}

const std::string::size_type mrt::utf8_length(const std::string &str) {
	std::string::size_type size = 0, str_size = str.size();
	for(std::string::size_type p = 0; p < str_size; ++p) {
		std::string::value_type c = str[p];
		if ((c & 0x80) == 0 || (c & 0xc0) != 0x80)
			++size;
	}
	return size;
}

unsigned mrt::utf8_iterate(const std::string &str, size_t &start) {
	if (start >= str.size()) 
		return 0;

	unsigned c0 = (unsigned char)str[start++];
	if (c0 <= 0x7f) 
		return c0;

	if (c0 == 0xc0 || c0 == 0xc1 || c0 >= 0xf5) 
		return '?';
	
	if (start >= str.size())
		return 0;
	
	unsigned c1 = (unsigned char)str[start++];
	if (c0 >= 0xc2 && c0 <= 0xdf) 
		return ((c0 & 0x1f) << 6) | (c1 & 0x3f);
	

	if (start >= str.size())
		return 0;
	unsigned c2 = (unsigned char)str[start++];
	if (c0 >= 0xe0 && c0 <= 0xef) 
		return ((c0 & 0x0f) << 12) | ((c1 & 0x3f) << 6) | (c2 & 0x3f);
	
	if (start >= str.size())
		return 0;
	unsigned c3 = (unsigned char)str[start++];
	if (c0 >= 0xf0 && c0 <= 0xf4) 
		return ((c0 & 0x07) << 18) | ((c1 & 0x3f) << 12) | ((c2 & 0x3f) << 6) | (c3 & 0x3f);

	return '?';
}


void mrt::utf8_add_wchar(std::string &str, unsigned wchar) {
	if (wchar <= 0x7f) {
		str += (char)wchar;
	} else if (wchar <= 0x7ff) {
		str += (char) ((wchar >> 6) | 0xc0);
		str += (char) ((wchar & 0x3f) | 0x80);
	} else if (wchar <= 0xffff) {
		str += (char)((wchar >> 12) | 0xe0);
		str += (char)(((wchar & 0x0fc0) >> 6) | 0x80);
		str += (char)( (wchar & 0x003f) | 0x80);
	} else if (wchar <= 0x10ffff) {
		str += (char)((wchar >> 18) | 0xf0);
		str += (char)(((wchar & 0x03f000) >> 12) | 0x80);
		str += (char)(((wchar & 0x000fc0) >> 6) | 0x80);
		str += (char)( (wchar & 0x00003f) | 0x80);
	} else 
		str += '?';
}

const size_t mrt::utf8_backspace(std::string &str, size_t pos) {
	if (str.empty())
		return 0;
	if (pos > str.size())
		pos = str.size();

	int p;
	for(p = (int)pos - 1; (p >= 0) && ((str[p] & 0xc0) == 0x80); --p) {}
	if (p >= 0) {
		std::string right;
		if (pos < str.size())
			right = str.substr(pos);
		str = ((p > 0)?str.substr(0, p):std::string()) + right;
		return p;
	} else {
		str.clear(); //p < 0
		return 0;
	}
}

const size_t mrt::utf8_left(const std::string &str, const size_t pos) {
	if (pos == 0 || str.empty())
		return 0;

	int p;
	for(p = (int)pos - 1; p >= 0 && (str[p] & 0xc0) == 0x80; --p) {}
	return (p > 0)? p: 0;	
}

const size_t mrt::utf8_right(const std::string &str, const size_t pos) {
	if (str.empty())
		return 0;

	size_t p;
	for(p = pos + 1; p < str.size() && (str[p] & 0xc0) == 0x80; ++p) {}
	return p >= str.size()? str.size(): p;
}

void MRTAPI mrt::utf8_resize(std::string &str, const size_t max) {
	std::string::size_type size = 0, str_size = str.size();
	std::string::size_type p;
	for(p = 0; p < str_size; ++p) {
		std::string::value_type c = str[p];
		if ((c & 0x80) == 0 || (c & 0xc0) != 0x80) {
			++size;
			if (size > max)
				break;
		}
	}
	str.resize(p);
}
