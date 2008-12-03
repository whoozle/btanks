#ifndef MRT_HASH_COMPAT_H__
#define MRT_HASH_COMPAT_H__

//fixme: find better way to determine stlport
#include <string>

#ifdef _STLP_BEGIN_NAMESPACE

//stlport 

#	include <hash_set>
#	include <hash_map>

#define MRT_HASH_MAP std::hash_map
#define MRT_HASH_SET std::hash_set

#elif defined(__GNUG__)

#	include <ext/hash_set>
#	include <ext/hash_map>

#define MRT_HASH_MAP __gnu_cxx::hash_map
#define MRT_HASH_SET __gnu_cxx::hash_set

#elif  defined(_MSC_VER)

#	include <hash_map>
#	include <hash_set>

#define MRT_HASH_MAP stdext::hash_map
#define MRT_HASH_SET stdext::hash_set

#else

#	error Unsupported compiler

#endif

#endif
