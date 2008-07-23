#ifndef __CORE_EXCEPTION_H__
#define __CORE_EXCEPTION_H__

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
	void add_message(const char *file, const int line);
	void add_message(const std::string &msg);
	virtual const std::string get_custom_message();
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
		const std::string get_custom_message(); \
		virtual ~name() throw(); \
	} 

#define DERIVE_EXCEPTION_NO_DEFAULT(export, name, ctor, data) \
	class export name : public mrt::Exception { \
		public: \
		name ctor; \
		const std::string get_custom_message(); \
		virtual ~name() throw(); \
		private: \
		data \
	} 

#define throw_generic(name, str) { name e; e.add_message(__FILE__, __LINE__); e.add_message(mrt::format_string str); e.add_message(e.get_custom_message()); throw e; }
#define throw_generic_no_default(name, str, args) { name e args; e.add_message(__FILE__, __LINE__); e.add_message(mrt::format_string str); e.add_message(e.get_custom_message()); throw e; }
#define throw_ex(str) throw_generic(mrt::Exception, str)

#define TRY try 

#if defined _WINDOWS && defined DEBUG
#	define CATCH_D(where, code) /* bye bye */
#else
#	define CATCH_D(where, code) catch(...) {\
		LOG_ERROR(("%s: unknown error", where));\
		code;\
		}
#endif

#define CATCH_NOD(where, code)  catch(const char * _e) {\
		LOG_ERROR(("%s: (const char*) %s", where, _e)); \
		code;\
	} catch(const std::exception &_e) {\
		LOG_ERROR(("%s: %s", where, _e.what())); \
		code;\
	} 

#define CATCH(where, code) \
	CATCH_NOD(where, code) \
	CATCH_D(where, code)

#endif
