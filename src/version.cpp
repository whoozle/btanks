#include "version.h"

const std::string& getVersion() {
	static std::string version("0.3." VERSION);
	return version;
}
