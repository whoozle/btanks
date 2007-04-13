#ifndef __STACKVM_LOGGER_H__
#define __STACKVM_LOGGER_H__
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

#include "singleton.h"
#include "fmt.h"
#include <stdio.h>
#include "export.h"

#define LL_DEBUG 0
#define LL_NOTICE 1
#define LL_WARN 6
#define LL_ERROR 7

namespace mrt {

class MRTAPI ILogger {
public:
	DECLARE_SINGLETON(ILogger);
	
	ILogger();
	virtual ~ILogger();

	void assign(const std::string &file);
	void close();

	void setLogLevel(const int level);
	const char * getLogLevelName(const int level);

	void log(const int level, const char *file, const int line, const std::string &str);
	const unsigned getLinesCounter() const { return _lines; }

private:
	int _level;
	unsigned _lines;
	
	FILE *fd;
};

SINGLETON(Logger, ILogger);
}

#define LOG_DEBUG(msg) mrt::ILogger::get_instance()->log(LL_DEBUG, __FILE__, __LINE__, mrt::formatString msg)
#define LOG_NOTICE(msg) mrt::ILogger::get_instance()->log(LL_NOTICE, __FILE__, __LINE__, mrt::formatString msg)
#define LOG_WARN(msg)  mrt::ILogger::get_instance()->log(LL_WARN, __FILE__, __LINE__, mrt::formatString msg)
#define LOG_ERROR(msg) mrt::ILogger::get_instance()->log(LL_ERROR, __FILE__, __LINE__, mrt::formatString msg)

#endif

