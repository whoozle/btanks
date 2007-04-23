#ifndef __CORE_EXCEPTION_H__
#define __CORE_EXCEPTION_H__
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

#include <stdio.h>
#include <string>
#include <exception>
#include "fmt.h"
#include "logger.h"
#include "export_mrt.h"

namespace mrt {

class MRTAPI Exception : public std::exception {
public:
	Exception();
	void addMessage(const char *file, const int line);
	void addMessage(const std::string &msg);
	virtual const std::string getCustomMessage();
	virtual const char* what() const throw();
	virtual ~Exception() throw();
private:
	std::string _error;
};

}

#define DERIVE_EXCEPTION(export, name) \
	class export name : public mrt::Exception { \
		public: \
		name(); \
		const std::string getCustomMessage(); \
		virtual ~name() throw(); \
	} 

#define DERIVE_EXCEPTION_NO_DEFAULT(export, name, ctor, data) \
	class export name : public mrt::Exception { \
		public: \
		name ctor; \
		const std::string getCustomMessage(); \
		virtual ~name() throw(); \
		private: \
		data \
	} 

#define throw_generic(name, str) { name e; e.addMessage(__FILE__, __LINE__); e.addMessage(mrt::formatString str); e.addMessage(e.getCustomMessage()); throw e; }
#define throw_generic_no_default(name, str, args) { name e args; e.addMessage(__FILE__, __LINE__); e.addMessage(mrt::formatString str); e.addMessage(e.getCustomMessage()); throw e; }
#define throw_ex(str) throw_generic(mrt::Exception, str)

#define TRY try 

#if defined WIN32 && defined DEBUG
#	define CATCH_D(where, code) /* bye bye */
#else
#	define CATCH_D(where, code) catch(...) {\
		LOG_ERROR(("%s: unknown error", where));\
		code;\
		}
#endif

#define CATCH(where, code)  catch(const char * _e) {\
		LOG_ERROR(("%s: (const char*) %s", where, _e)); \
		code;\
	} catch(const std::exception &_e) {\
		LOG_ERROR(("%s: %s", where, _e.what())); \
		code;\
	} CATCH_D(where, code)

#endif
