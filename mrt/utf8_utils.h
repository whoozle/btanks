#ifndef MRT_UTF8_UTILS_H__
#define MRT_UTF8_UTILS_H__

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


#include "export_mrt.h"
#include <string>

namespace mrt {
	const std::string::size_type MRTAPI utf8_length(const std::string &str);
	void MRTAPI utf8_add_wchar(std::string &str, const int wchar);
	const size_t MRTAPI utf8_backspace(std::string &str, size_t pos);
	const size_t MRTAPI utf8_left(const std::string &str, const size_t pos);
	const size_t MRTAPI utf8_right(const std::string &str, const size_t pos);
	void MRTAPI utf8_resize(std::string &str, const size_t max);

	unsigned MRTAPI utf8_iterate(const std::string &str, size_t &start); //0 = end
	unsigned MRTAPI wchar2lower(unsigned ch);
	unsigned MRTAPI wchar2upper(unsigned ch);
}

#endif

