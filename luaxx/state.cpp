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

static void check_error(lua_State * state, const int err) {
	switch(err) {
		case 0: 
			return;
		case LUA_ERRRUN:
		case LUA_ERRERR:
		case LUA_ERRSYNTAX: {
			std::string error = lua_tostring(state, -1);
			lua_pop(state, 1);
			throw_ex(("lua error[%d]: %s", err, error.c_str()));
			}
		case LUA_ERRMEM:
			throw_ex(("lua is out of memory"));
		default: 
			throw_ex(("unknown lua error[%d]", err));
	}
}

void State::loadFile(const std::string &fname) {
	int err = luaL_loadfile(state, fname.c_str());
	if (err == LUA_ERRFILE)
		throw_ex(("file '%s' not found", fname.c_str()));
	check_error(state, err);
}

void State::call(const int nargs, const int nresults) const {
	int err = lua_pcall(state, nargs, nresults, 0);
	//if (err == LUA_ERRRUN);
	check_error(state, err);
}

State::State() {
	state = lua_newstate(l_alloc, this);
	if (state == NULL)
		throw_ex(("cannot create lua interpreter"));
}

void State::clear() {
	lua_close(state);

	state = lua_newstate(l_alloc, this);
	if (state == NULL)
		throw_ex(("cannot create lua interpreter"));
}

State::~State() {
	lua_close(state);
}


