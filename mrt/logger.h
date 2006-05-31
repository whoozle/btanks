#ifndef __STACKVM_LOGGER_H__
#define __STACKVM_LOGGER_H__

#include "singleton.h"
#include "fmt.h"

#define LL_DEBUG 0
#define LL_WARN 6
#define LL_ERROR 7

namespace mrt {

class ILogger {
public:
	DECLARE_SINGLETON(ILogger);
	
	ILogger();
	virtual ~ILogger();

	void setLogLevel(const int level);
	const char * getLogLevelName(const int level);

	void log(const int level, const char *file, const int line, const std::string &str);

private:
	int _level;
};

SINGLETON(Logger, ILogger);
}

#define LOG_DEBUG(msg) mrt::ILogger::get_instance()->log(LL_DEBUG, __FILE__, __LINE__, mrt::formatString msg)
#define LOG_WARN(msg)  mrt::ILogger::get_instance()->log(LL_WARN, __FILE__, __LINE__, mrt::formatString msg)
#define LOG_ERROR(msg) mrt::ILogger::get_instance()->log(LL_ERROR, __FILE__, __LINE__, mrt::formatString msg)

#endif

