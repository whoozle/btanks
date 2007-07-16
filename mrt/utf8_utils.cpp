#include "utf8_utils.h"

const std::string::size_type mrt::utf8_length(const std::string &str) {
	std::string::size_type size;
	for(std::string::size_type p = 0; p < str.size(); ++p) {
		std::string::value_type c = str[p];
		if ((c & 0x80) == 0 || (c & 0xc0) != 0x80)
			++size;
	}
	return size;
}
