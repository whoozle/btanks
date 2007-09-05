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
#include "exception.h"
#include <stdarg.h>

using namespace mrt;

Exception::Exception() : _error() {}
Exception::~Exception() throw() {}

const std::string Exception::getCustomMessage() { return std::string(); }
const char* Exception::what() const throw() { return _error.c_str(); }


void Exception::addMessage(const char * file, const int line) {
	char buf[1024];
	size_t n = snprintf(buf, sizeof(buf), "[%s:%d]", file, line);
	_error = std::string(buf, n);
}

void Exception::addMessage(const std::string &msg) {
	if (msg.size() == 0) return;
	
	_error += ": " + msg;
}
