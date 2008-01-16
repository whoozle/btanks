#ifndef __STACKVM__FMT_H__
#define __STACKVM__FMT_H__

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

#include <string>
#include <vector>
#include "export_mrt.h"

#if !(defined(__GNUC__) || defined(__GNUG__) || defined(__attribute__))
#	define __attribute__(p) /* nothing */
#endif

namespace mrt {


const std::string MRTAPI formatString(const char *fmt, ...) __attribute__((format(printf, 1, 2)));
void MRTAPI trim(std::string &str, const std::string chars = "\t\n\r ");
void MRTAPI split(std::vector<std::string> & result, const std::string &str, const std::string &delimiter, const size_t limit = 0);
void MRTAPI join(std::string &result, const std::vector<std::string>& array, const std::string &delimiter, const size_t limit = 0);
void MRTAPI replace(std::string &str, const std::string &from, const std::string &to, const size_t limit = 0);

void MRTAPI toUpper(std::string &str);
void MRTAPI toLower(std::string &str);

}

#endif

