#ifndef MRT_EXPORT_MACRO_H__
#define MRT_EXPORT_MACRO_H__

// Shared library support
#ifdef WIN32
#	define DLLIMPORT __declspec(dllimport)
#	define DLLEXPORT __declspec(dllexport)
#	define DLLDLLLOCAL
#	define DLLDLLPUBLIC
#else
#	define DLLIMPORT
#	ifdef GCC_HASCLASSVISIBILITY
#		define DLLIMPORT __attribute__ ((visibility("default")))
#		define DLLEXPORT __attribute__ ((visibility("default")))
#		define DLLDLLLOCAL __attribute__ ((visibility("hidden")))
#		define DLLDLLPUBLIC __attribute__ ((visibility("default")))
#	else
#		define DLLIMPORT
#		define DLLEXPORT
#		define DLLDLLLOCAL
#		define DLLDLLPUBLIC
#	endif
#endif

#endif
