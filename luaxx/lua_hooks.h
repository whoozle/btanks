#ifndef BTANKS_LUA_HOOKS_H__
#define BTANKS_LUA_HOOKS_H__

#include <string>
#include "luaxx/state.h"

class LuaHooks {
public: 
	void load(const std::string &name);
	void clear();
	void on_tick(const float dt);
	void call(const std::string &method);
private: 
	bool check_function(const std::string &name);
	luaxx::State state;
	
	bool has_on_tick, has_on_death;
};

#endif

