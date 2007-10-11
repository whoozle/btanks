#include "luaxx/state.h"
#include "mrt/exception.h"
#include <stdlib.h>
#include "mrt/chunk.h"
#include "mrt/file.h"

using namespace luaxx;

static void *l_alloc (void *ud, void *ptr, size_t osize, size_t nsize) {
	if (nsize == 0) {
		free(ptr);
		return NULL;
	} else {
		 return realloc(ptr, nsize);
	}
}

static void check_error(const int err) {
	switch(err) {
		case 0: 
			return;
		case LUA_ERRRUN:
			throw_ex(("lua runtime error"));
		case LUA_ERRMEM:
			throw_ex(("lua is out of memory"));
		case LUA_ERRERR:
			throw_ex(("error calling lua's error handler"));
		case LUA_ERRSYNTAX: 
			throw_ex(("lua syntax error"));
		default: 
			throw_ex(("unknown lua error[%d]", err));
	}
}

struct ReaderStub {
	bool read; 
	mrt::Chunk data;
};

static const char * l_reader (lua_State *L, void *ud, size_t *sz) {
	ReaderStub *info = reinterpret_cast<ReaderStub *>(ud);
	if (info == NULL || info->read)
		return NULL;
	*sz = info->data.getSize();
	info->read = true;
	return (const char *)info->data.getPtr();
}

void State::loadFile(const std::string &fname) {
	ReaderStub stub;
	mrt::File file;
	file.open(fname, "rb");
	file.readAll(stub.data);
	file.close();
	stub.read = false;
	int err = lua_load(state, l_reader, &stub, fname.c_str());
	check_error(err);
}

State::State() {
	state = lua_newstate(l_alloc, this);
	if (state == NULL)
		throw_ex(("cannot create lua interpreter"));
}

State::~State() {
	lua_close(state);
}


