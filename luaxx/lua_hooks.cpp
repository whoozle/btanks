#include "lua_hooks.h"
#include "mrt/logger.h"

void LuaHooks::load(const std::string &name) {
	LOG_DEBUG(("loading lua code from %s...", name.c_str()));
	state.loadFile(name);
	state.call(0, LUA_MULTRET);
	
	has_on_timer = check_function("on_timer");
	has_on_timer = check_function("on_death");
}

bool LuaHooks::check_function(const std::string &name) {
	lua_getglobal(state, name.c_str());
	int top = lua_gettop(state);
	LOG_DEBUG(("top: %d, string: %s", top, lua_typename(state, lua_type(state, -1))));
	LOG_DEBUG(("checking for function: %s: %c", name.c_str(), false?'+':'-'));
	return false;
}

void LuaHooks::on_timer(const float dt) {
	
}
