#ifndef MRT_EXPORT
#define MRT_EXPORT

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

#ifdef BUILD_DLL
#	ifdef DLL_EXPORTS
#		define DLLAPI DLLEXPORT
#	else
#		define DLLAPI DLLIMPORT
#	endif
#else
#	define DLLAPI
#endif // BUILD_DLL

#endif
