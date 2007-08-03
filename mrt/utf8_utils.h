#ifndef MRT_UTF8_UTILS_H__
#define MRT_UTF8_UTILS_H__

#include "export_mrt.h"
#include <string>

namespace mrt {
	const std::string::size_type MRTAPI utf8_length(const std::string &str);
	void MRTAPI utf8_add_wchar(std::string &str, const int wchar);
	const size_t MRTAPI utf8_backspace(std::string &str, size_t pos);
	const size_t MRTAPI utf8_left(const std::string &str, const size_t pos);
	const size_t MRTAPI utf8_right(const std::string &str, const size_t pos);
}

#endif

