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
