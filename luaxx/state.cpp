#include "luaxx/state.h"
#include "mrt/exception.h"
#include <stdlib.h>
#include <lauxlib.h>

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

void State::loadFile(const std::string &fname) {
	int err = luaL_loadfile(state, fname.c_str());
	if (err == LUA_ERRFILE)
		throw_ex(("file '%s' not found", fname.c_str()));
	check_error(err);
}

void State::call(const int nargs, const int nresults) const {
	int err = lua_pcall(state, nargs, nresults, 0);
	//if (err == LUA_ERRRUN);
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


