#ifndef BTANKS_LUA_HOOKS_H__
#define BTANKS_LUA_HOOKS_H__

#include <string>
#include "luaxx/state.h"

class LuaHooks {
public: 
	LuaHooks();
	void load(const std::string &name);
	void clear();

	void on_tick(const float dt);
	void on_load();
	const bool on_spawn(const std::string &classname, const std::string &animation, const std::string &property); 

	void call(const std::string &method);
	void call1(const std::string &method, const int id);
private: 
	bool check_function(const std::string &name);
	luaxx::State state;
	
	bool has_on_tick, has_on_spawn, has_on_load;
};

#endif

