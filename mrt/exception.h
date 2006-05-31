#ifndef __CORE_EXCEPTION_H__
#define __CORE_EXCEPTION_H__

#include <stdio.h>
#include <string>
#include <exception>
#include "fmt.h"
#include "logger.h"

namespace mrt {

class Exception : public std::exception {
public:
	Exception();
	void addMessage(const char *file, const int line);
	void addMessage(const std::string &msg);
	virtual const std::string getCustomMessage() { return ""; }
	virtual const char* what() const throw() { return _error.c_str(); }
	virtual ~Exception() throw() {};
private:
	std::string _error;
};

}

#define DERIVE_EXCEPTION(name) \
	class name : public mrt::Exception { \
		public: \
		name(); \
		const std::string getCustomMessage(); \
		virtual ~name() throw(); \
	} 

#define throw_generic(name, str) { name e; e.addMessage(__FILE__, __LINE__); e.addMessage(mrt::formatString str); e.addMessage(e.getCustomMessage()); throw e; }
#define throw_ex(str) throw_generic(mrt::Exception, str)

#define TRY try 
#define CATCH(where, code)  catch(const std::exception &_e) {\
	LOG_ERROR(("%s: %s", where, _e.what())); \
	code;\
	} catch(...) {\
	LOG_ERROR(("%s: unknown error", where));\
	code;\
	}\

#endif
