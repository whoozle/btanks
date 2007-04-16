#ifndef __STACKVM__FMT_H__
#define __STACKVM__FMT_H__
/* M-Runtime for c++
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

void MRTAPI toUpper(std::string &str);
void MRTAPI toLower(std::string &str);

}

#endif

