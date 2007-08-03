#include "utf8_utils.h"

const std::string::size_type mrt::utf8_length(const std::string &str) {
	std::string::size_type size = 0, str_size = str.size();
	for(std::string::size_type p = 0; p < str_size; ++p) {
		std::string::value_type c = str[p];
		if ((c & 0x80) == 0 || (c & 0xc0) != 0x80)
			++size;
	}
	return size;
}

void mrt::utf8_add_wchar(std::string &str, const int wchar) {
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
	if (p > 0) {
		std::string right;
		if (pos < str.size())
			right = str.substr(pos);
		str = str.substr(0, p) + right;
		return p;
	} else {
		str.clear(); //p <= 0
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
