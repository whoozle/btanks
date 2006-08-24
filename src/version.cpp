#include "version.h"
#ifdef WIN32
#pragma comment(exestr, "Battle Tanks (c)2006 Battle tanks team. version 0.3." VERSION)
#pragma comment( user, "Compiled on " __DATE__ " at " __TIME__ ) 
#endif

const std::string& getVersion() {
	static std::string version("0.3." VERSION);
	return version;
}
