#ifndef MRT_UTF8_UTILS_H__
#define MRT_UTF8_UTILS_H__

#include "export_mrt.h"
#include <string>

namespace mrt {
	const std::string::size_type MRTAPI utf8_length(const std::string &str);
}

#endif

