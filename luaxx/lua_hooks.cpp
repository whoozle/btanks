#include "lua_hooks.h"
#include "special_owners.h"
#include "mrt/logger.h"
#include "object.h"
#include "world.h"
#include "resource_manager.h"
#include <assert.h>
#include <stdexcept>

static int lua_hooks_print(lua_State *L) {
	int n = lua_gettop(L);
	std::string str;
	for (int i = 1; i <= n; i++) {
		const char *v = lua_tostring(L, i);
		str += v?v: "(nil)";
	}
	LOG_DEBUG(("[lua] %s", str.c_str()));
	
	return 0;
}

static int lua_hooks_spawn(lua_State *L) {
	int n = lua_gettop(L);
	if (n < 4) {
		lua_pushstring(L, "spawn() requires at least 4 arguments: classname, animation");
		lua_error(L);
	}
	try {
		const char *classname = lua_tostring(L, 1);
		if (classname == NULL)
			throw std::runtime_error("spawn: first argument must be string");
		const char *animation = lua_tostring(L, 2);
		if (animation == NULL)
			throw std::runtime_error("spawn: second argument must be string");
		int x = lua_tointeger(L, 3);
		int y = lua_tointeger(L, 4);
		
		Object *o = ResourceManager->createObject(classname, animation);
	//	if (z) 
	//		o->setZ(z, true);
		o->addOwner(OWNER_MAP);

	//	if (dir) 
	//		o->setDirection(dir);
	
		World->addObject(o, v2<float>(x, y));
	} catch(const std::exception &e) {
		lua_pushstring(L, e.what());
		lua_error(L);
	} catch(...) {
		lua_pushstring(L, "unknown exception");
		lua_error(L);
	}
	return 0;
}

void LuaHooks::load(const std::string &name) {
	LOG_DEBUG(("loading lua code from %s...", name.c_str()));
	state.loadFile(name);
	
	lua_register(state, "print", lua_hooks_print);
	lua_register(state, "spawn", lua_hooks_spawn);
	
	state.call(0, LUA_MULTRET);
	
	has_on_tick = check_function("on_tick");
	has_on_death = check_function("on_death");
}

bool LuaHooks::check_function(const std::string &name) {
	int top0 = lua_gettop(state);
	
	lua_getglobal(state, name.c_str());
	bool r = !(lua_isnoneornil(state, -1));
	
	LOG_DEBUG(("checking for function: %s: %c", name.c_str(), r?'+':'-'));
	lua_pop(state, 1);

	assert(lua_gettop(state) == top0);
	return r;
}

void LuaHooks::on_tick(const float dt) {
	if (!has_on_tick)
		return;
	
	int top0 = lua_gettop(state);
	
	lua_getglobal(state, "on_tick");
	lua_pushnil(state);
	lua_pushnumber(state, dt);

	state.call(2, 0);

	assert(lua_gettop(state) == top0);
}
