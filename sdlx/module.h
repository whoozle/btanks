#ifndef SDLX_MODULE_H__
#define SDLX_MODULE_H__

#include "export_sdlx.h"
#include <string>

namespace sdlx {

class SDLXAPI Module {
public: 
	Module();
	void load(const std::string &name);
	void *sym(const std::string &name) const;
	void leak();
	void unload();
	~Module();
private:
	void * handle;	
};

}

#endif

