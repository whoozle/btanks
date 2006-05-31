#ifndef __STACKVM__FMT_H__
#define __STACKVM__FMT_H__

#include <string>
#include <inttypes.h>

#if !(defined(__GNUC__) || defined(__GNUG__) || defined(__attribute__))
#	define __attribute__(p) /* nothing */
#endif

namespace mrt {


const std::string formatString(const char *fmt, ...) __attribute__((format(printf, 1, 2)));
void trim(std::string &str, const std::string chars = "\t\n\r ");

}

#endif

