#ifndef BTANKS_LUA_HOOKS_H__
#define BTANKS_LUA_HOOKS_H__

#include <string>
#include "luaxx/state.h"

class LuaHooks {
public: 
	void load(const std::string &name);
	void on_timer(const float dt);
private: 
	lua_CFunction get_function(const std::string &name);
	luaxx::State state;
	
	lua_CFunction lua_on_timer, lua_on_death;
};

#endif

