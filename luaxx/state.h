#ifndef LUA_STATE_H__
#define LUA_STATE_H__

#define LUA_REENTRANT
#include <lua.hpp>
#include <string>

namespace luaxx {
class State {
public: 
	State();
	void loadFile(const std::string &fname);
	~State();

private: 
	lua_State * state;
};
}

#endif
