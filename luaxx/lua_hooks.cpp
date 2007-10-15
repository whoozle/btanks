#include "lua_hooks.h"
#include "mrt/logger.h"

void LuaHooks::load(const std::string &name) {
	LOG_DEBUG(("loading lua code from %s...", name.c_str()));
	state.loadFile(name);
	lua_on_timer = get_function("on_timer");
	lua_on_death = get_function("on_death");
}

lua_CFunction LuaHooks::get_function(const std::string &name) {
	lua_getglobal((lua_State*)state, name.c_str());
	lua_CFunction func = lua_tocfunction((lua_State*)state, 0);
	LOG_DEBUG(("checking for function: %s: %c", name.c_str(), func?'+':'-'));
	return func;
}

void LuaHooks::on_timer(const float dt) {
	
}
