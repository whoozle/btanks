#include "lua_hooks.h"
#include "special_owners.h"
#include "mrt/logger.h"
#include "object.h"
#include "world.h"
#include "resource_manager.h"
#include "game_monitor.h"
#include <assert.h>
#include <stdexcept>

#define LUA_TRY try
#define LUA_CATCH(where) catch(const std::exception &e) {\
		lua_pushstring(L, e.what());\
		lua_error(L);\
		return 0;\
	} catch(...) {\
		lua_pushstring(L, "unknown exception");\
		lua_error(L);\
		return 0;\
	}


static int lua_hooks_print(lua_State *L) {
LUA_TRY {
	int n = lua_gettop(L);
	std::string str;
	for (int i = 1; i <= n; i++) {
		const char *v = lua_tostring(L, i);
		str += v?v: "(nil)";
	}
	LOG_DEBUG(("[lua] %s", str.c_str()));
	
	return 0;
} LUA_CATCH("lua_hooks_print")
}

static int lua_hooks_object_exists(lua_State *L) {
LUA_TRY {
	int n = lua_gettop(L);
	if (n < 1) {
		lua_pushstring(L, "object_exists requires object id");
		lua_error(L);
		return 0;
	}
	int id = lua_tointeger(L, 1);
	lua_pushboolean(L, World->getObjectByID(id)?1:0);
	return 1;
} LUA_CATCH("lua_hooks_object_exists")
}

static int lua_hooks_show_item(lua_State *L) {
	LUA_TRY {
		int n = lua_gettop(L);
		if (n < 1) {
			lua_pushstring(L, "show_item requires item's property as first argument");
			lua_error(L);
			return 0;
		}
		const char *prop = lua_tostring(L, 1);
		if (prop == NULL) {
			lua_pushstring(L, "show_item's first argument must be string");
			lua_error(L);
			return 0;
		}
		GameItem &item = GameMonitor->find(prop);
		if (item.hidden)
			item.respawn();
		
		lua_pushinteger(L, item.id);
		return 1;
	} LUA_CATCH("lua_hooks_show_item")
}

static int lua_hooks_hide_item(lua_State *L) {
	LUA_TRY {
		int n = lua_gettop(L);
		if (n < 1) {
			lua_pushstring(L, "hide_item requires item's property as first argument");
			lua_error(L);
			return 0;
		}
		const char *prop = lua_tostring(L, 1);
		if (prop == NULL) {
			lua_pushstring(L, "hide_item's first argument must be string");
			lua_error(L);
			return 0;
		}
		GameItem &item = GameMonitor->find(prop);
		item.hidden = true;

		Object *o = World->getObjectByID(item.id);
		if (o != NULL) {
			//silently kill 
			o->Object::emit("death", NULL);
		}
		
		return 0;
	} LUA_CATCH("lua_hooks_show_item")
}

static int lua_hooks_spawn(lua_State *L) {
	LUA_TRY {
		int n = lua_gettop(L);
		if (n < 4) {
			lua_pushstring(L, "spawn() requires at least 4 arguments: classname, animation");
			lua_error(L);
			return 0;
		}
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
		lua_pushinteger(L, o->getID());
		return 1;
	} LUA_CATCH("lua_hooks_spawn")
}

void LuaHooks::load(const std::string &name) {
	LOG_DEBUG(("loading lua code from %s...", name.c_str()));
	state.loadFile(name);
	
	lua_register(state, "print", lua_hooks_print);
	lua_register(state, "spawn", lua_hooks_spawn);
	lua_register(state, "object_exists", lua_hooks_object_exists);
	lua_register(state, "show_item", lua_hooks_show_item);
	lua_register(state, "hide_item", lua_hooks_hide_item);
	
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
	lua_pushnumber(state, dt);

	state.call(1, 0);

	assert(lua_gettop(state) == top0);
}
