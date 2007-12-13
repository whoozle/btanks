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
	void open();
	void call(const int nargs, const int nresults) const;
	~State();
	
	inline operator lua_State*() { return state; } 
	void clear();

private: 
	void init();
	lua_State * state;
};
}

#endif
