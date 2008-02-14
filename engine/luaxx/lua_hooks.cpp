#include "lua_hooks.h"
#include "special_owners.h"
#include "mrt/logger.h"
#include "mrt/random.h"
#include "object.h"
#include "world.h"
#include "resource_manager.h"
#include "game_monitor.h"
#include "player_manager.h"
#include "player_slot.h"
#include "tmx/map.h"
#include "sound/mixer.h"
#include "game.h"
#include <assert.h>
#include <stdexcept>
#include "var.h"
#include "config.h"

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

static std::string next_map;

const std::string & LuaHooks::getNextMap() { return next_map; }
void LuaHooks::resetNextMap() { next_map.clear(); }


static int lua_hooks_print(lua_State *L) {
LUA_TRY {
	int n = lua_gettop(L);
	std::string str;
	for (int i = 1; i <= n; i++) {
		const char *v = lua_tostring(L, i);
		str += v?v: "(nil)";
		str += '\t';
	}
	LOG_DEBUG(("[lua] %s", str.c_str()));
	
	return 0;
} LUA_CATCH("lua_hooks_print")
}


static int lua_hooks_spawn_random(lua_State *L) {
LUA_TRY {
	int args = lua_gettop(L);
	if (args < 2) {
		lua_pushstring(L, "spawn_random requires object and animation");
		lua_error(L);
		return 0;		
	}
	const char *object = lua_tostring(L, 1), *animation = lua_tostring(L, 2);
	Object *obj = ResourceManager->createObject(object, animation);

	//Matrix<int> matrix;
	//World->getImpassabilityMatrix(matrix, obj, NULL);
	const Matrix<int> &matrix = Map->getImpassabilityMatrix(0);
		
	const v2<int> tile_size = Map->getPathTileSize();
	if (obj->size.is0())
		throw_ex(("object size must not be 0,0"));
		
	v2<int> obj_size = ((obj->size.convert<int>() - 1) / tile_size) + 1;
	LOG_DEBUG(("searching random %dx%d spot", obj_size.x, obj_size.y));
	
	int w = matrix.getWidth(), h = matrix.getHeight();
	std::vector<v2<int> > spots;
	for(int y = 0; y < h - obj_size.y + 1; ++y) 
		for(int x= 0; x < w - obj_size.x + 1; ++x) {
			for(int yy = 0; yy < obj_size.y; ++yy)
				for(int xx = 0; xx < obj_size.x; ++xx) {
					int im = matrix.get(y + yy, x + xx);
					if (im < 0 || im >= 100)
						goto skip;
				
				}
			spots.push_back(v2<int>(x, y));
		skip: ;
		}
		
	size_t n = spots.size();
	if (n == 0)
		throw_ex(("no spots found"));
		
	int idx = mrt::random(n);
	LOG_DEBUG(("found %u spots. get #%d", (unsigned)n, idx));
	v2<float> pos = (spots[idx] * tile_size).convert<float>();
		
	//LOG_DEBUG(("map : %s", matrix.dump().c_str()));
	obj_size = tile_size * obj_size / 2;
	World->addObject(obj, pos + obj_size.convert<float>() - obj->size / 2);
	
	lua_pushinteger(L, obj->getID());
	return 1;
} LUA_CATCH("lua_hooks_spawn_random")
}

static int lua_hooks_map_size(lua_State *L) {
LUA_TRY {
	v2<int> map_size = Map->getSize();
	lua_pushinteger(L, map_size.x);
	lua_pushinteger(L, map_size.y);
	return 2;
} LUA_CATCH("lua_hooks_map_size")
}

static int lua_hooks_load_map(lua_State *L) {
LUA_TRY {
	int n = lua_gettop(L);
	if (n < 1) {
		lua_pushstring(L, "load_map requires map name");
		lua_error(L);
		return 0;
	}
	const char * name = lua_tostring(L, 1);
	if (name == NULL) 
		throw_ex(("load_map's 1st argument is not a string"));
	next_map = name;
	return 0;
} LUA_CATCH("lua_hooks_load_map")
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
	const Object *o = World->getObjectByID(id);

	bool strict = (n >= 2)? lua_toboolean(L, 2) != 0: false;
		
	bool exists = o?!o->isDead():false;
	if (exists && !strict && o->getState() == "broken")
		exists = false;
		
	lua_pushboolean(L, exists?1:0);
	return 1;
} LUA_CATCH("lua_hooks_object_exists")
}

static int lua_hooks_object_property(lua_State *L) {
LUA_TRY {
	int n = lua_gettop(L);
	if (n < 2) {
		lua_pushstring(L, "object_property requires object id and property name");
		lua_error(L);
		return 0;
	}
	int id = lua_tointeger(L, 1);
	const Object *o = World->getObjectByID(id);
	if (o == NULL) {
		lua_pushnil(L);
		return 1;
	}
	std::string prop = lua_tostring(L, 2);
	if (prop == "classname") {
		lua_pushstring(L, o->classname.c_str());
		return 1;	
	} else if (prop == "registered_name") {
		lua_pushstring(L, o->registered_name.c_str());
		return 1;	
	} else if (prop == "animation") {
		lua_pushstring(L, o->animation.c_str());
		return 1;	
	} else if (prop == "hp") {
		lua_pushinteger(L, o->hp);
		return 1;	
	}

	lua_pushstring(L, mrt::formatString("object_property: unknown property %s", prop.c_str()).c_str());
	lua_error(L);
	return 0;
	
} LUA_CATCH("lua_hooks_object_property")	
}

static int lua_hooks_set_slot_property(lua_State *L) {
LUA_TRY {
	int n = lua_gettop(L);
	if (n < 3) {
		lua_pushstring(L, "set_slot_property requires object id, property name and property value");
		lua_error(L);
		return 0;
	}
	int id = lua_tointeger(L, 1);
	if (id < 1) 
		throw_ex(("slot #%d is invalid", id));
	PlayerSlot &slot =  PlayerManager->getSlot(id - 1);
		
	std::string prop = lua_tostring(L, 2);
	if (prop == "classname") {
		slot.classname = lua_tostring(L, 3);
		return 0;
	} else if (prop == "animation") {
		slot.animation = lua_tostring(L, 3);
		return 0;
	} else if (prop == "spawn_limit") {
		slot.spawn_limit = lua_tointeger(L, 3);
		return 0;
	}
	
	lua_pushstring(L, mrt::formatString("object_property: unknown property %s", prop.c_str()).c_str());
	lua_error(L);
	return 0;
	
} LUA_CATCH("lua_hooks_object_property")	
}

static int lua_hooks_slot_property(lua_State *L) {
LUA_TRY {
	int n = lua_gettop(L);
	if (n < 2) {
		lua_pushstring(L, "slot_property requires object id and property name");
		lua_error(L);
		return 0;
	}
	int id = lua_tointeger(L, 1);
	if (id < 1) 
		throw_ex(("slot #%d is invalid", id));
	PlayerSlot &slot =  PlayerManager->getSlot(id - 1);
		
	std::string prop = lua_tostring(L, 2);
	if (prop == "classname") {
		lua_pushstring(L, slot.classname.c_str());
		return 1;
	} else if (prop == "animation") {
		lua_pushstring(L, slot.animation.c_str());
		return 1;
	} else if (prop == "spawn_limit") {
		lua_pushinteger(L, slot.spawn_limit);
		return 1;
	} else if (prop == "id") {
		lua_pushinteger(L, slot.id);
		return 1;
	}
	
	lua_pushstring(L, mrt::formatString("object_property: unknown property %s", prop.c_str()).c_str());
	lua_error(L);
	return 0;
	
} LUA_CATCH("lua_hooks_object_property")	
}


static int lua_hooks_kill_object(lua_State *L) {
	LUA_TRY {
		int n = lua_gettop(L);
		if (n < 1) {
			lua_pushstring(L, "kill object requres object id as first argument");
			lua_error(L);
			return 0;
		}
		int id = lua_tointeger(L, 1);
		bool system = (n >= 2)? lua_toboolean(L, 2) != 0: false;
		
		Object *o = World->getObjectByID(id);
		if (o == NULL)
			return 0;
		
		if (system) {
			o->Object::emit("death", NULL);
		} else {
			o->emit("death", NULL);
		}
		return 0;
	} LUA_CATCH("lua_hooks_kill_object")
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
		if (item.hidden || World->getObjectByID(item.id) == NULL)
			item.respawn();
		
		lua_pushinteger(L, item.id);
		return 1;
	} LUA_CATCH("lua_hooks_show_item")
}

static int lua_hooks_kill_item(lua_State *L) {
	LUA_TRY {
		int n = lua_gettop(L);
		if (n < 1) {
			lua_pushstring(L, "kill_item requires item's property as first argument");
			lua_error(L);
			return 0;
		}
		const char *prop = lua_tostring(L, 1);
		if (prop == NULL) {
			lua_pushstring(L, "kill_item's first argument must be string");
			lua_error(L);
			return 0;
		}
		GameItem &item = GameMonitor->find(prop);
		Object *o = World->getObjectByID(item.id);
		if (o != NULL && !o->isDead())
			o->emit("death", NULL); 
		return 0;
	} LUA_CATCH("lua_hooks_kill_item")
}

static int lua_hooks_play_tune(lua_State *L) {
	LUA_TRY {
		int n = lua_gettop(L);
		if (n < 1) {
			lua_pushstring(L, "play_tune requre tune name");
			lua_error(L);
			return 0;
		}
		const char *name = lua_tostring(L, 1);
		if (name == NULL) {
			lua_pushstring(L, "tune name must be string");
			lua_error(L);
			return 0;
		}
		bool loop = true;
		if (n >= 2) {
			loop = lua_toboolean(L, 2) != 0;
		}
		
		Mixer->play(name, loop);
		return 0;
	} LUA_CATCH("lua_hooks_play_tune")
}

static int lua_hooks_reset_tune(lua_State *L) {
	LUA_TRY {
		Mixer->reset();
		return 0;
	} LUA_CATCH("lua_hooks_reset_tune")
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
		item.kill();

		return 0;
	} LUA_CATCH("lua_hooks_hide_item")
}

static int lua_hooks_item_exists(lua_State *L) {
	LUA_TRY {
		int n = lua_gettop(L);
		if (n < 1) {
			lua_pushstring(L, "item_exists requires item's property as first argument");
			lua_error(L);
			return 0;
		}
		const char *prop = lua_tostring(L, 1);
		if (prop == NULL) {
			lua_pushstring(L, "item_exists' first argument must be string");
			lua_error(L);
			return 0;
		}
		bool strict = (n >= 2)? lua_toboolean(L, 2) != 0: false;
		
		GameItem &item = GameMonitor->find(prop);
		const Object *o = World->getObjectByID(item.id);
		
		bool exists = o?!o->isDead():false;
		if (exists && !strict && o->getState() == "broken")
			exists = false;
		
		lua_pushboolean(L, exists?1:0);
		return 1;
	} LUA_CATCH("lua_hooks_item_exists")
}


static int lua_hooks_spawn(lua_State *L) {
	LUA_TRY {
		int n = lua_gettop(L);
		if (n < 4) {
			lua_pushstring(L, "spawn() requires at least 4 arguments: classname, animation, x, y");
			lua_error(L);
			return 0;
		}
		const char *classname = lua_tostring(L, 1);
		if (classname == NULL) {
			lua_pushstring(L, "spawn: first argument must be string");
			lua_error(L);
			return 0;		
		}
		const char *animation = lua_tostring(L, 2);
		if (animation == NULL) {
			lua_pushstring(L, "spawn: first argument must be string");
			lua_error(L);
			return 0;		
		}

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

static int lua_hooks_game_over(lua_State *L) {
	LUA_TRY {
		int n = lua_gettop(L);
		if (n < 4) {
			lua_pushstring(L, "game_over() requires at least 4 arguments: area, message, time and win");
			lua_error(L);
			return 0;
		}

		const char *area = lua_tostring(L, 1);
		if (area == NULL) {
			lua_pushstring(L, "game_over: first argument must be string");
			lua_error(L);
			return 0;		
		}

		const char *message = lua_tostring(L, 2);
		if (message == NULL) {
			lua_pushstring(L, "game_over: second argument must be string");
			lua_error(L);
			return 0;		
		}
		lua_Number time = lua_tonumber(L, 3);
		bool win = lua_toboolean(L, 4) != 0;
		GameMonitor->gameOver(area, message, (float)time, win);
	} LUA_CATCH("lua_hooks_game_over")
	return 0;		
}

static int lua_hooks_set_timer(lua_State *L) {
	LUA_TRY {
		int n = lua_gettop(L);
		if (n < 4) {
			lua_pushstring(L, "set_timer: requires at least 4 arguments: area, message, time and win");
			lua_error(L);
			return 0;
		}

		const char *area = lua_tostring(L, 1);
		if (area == NULL) {
			lua_pushstring(L, "set_timer: first argument must be string");
			lua_error(L);
			return 0;		
		}

		const char *message = lua_tostring(L, 2);
		if (message == NULL) {
			lua_pushstring(L, "set_timer: second argument must be string");
			lua_error(L);
			return 0;		
		}
		
		lua_Number time = lua_tonumber(L, 3);
		bool win = lua_toboolean(L, 4) != 0;
		GameMonitor->setTimer(area, message, (float)time, win);
	} LUA_CATCH("lua_hooks_set_timer")
	return 0;		
}

static int lua_hooks_reset_timer(lua_State *L) {
	LUA_TRY {
		GameMonitor->resetTimer();
	} LUA_CATCH("lua_hooks_set_timer")
	return 0;		
}
	

static int lua_hooks_display_message(lua_State *L) {
	LUA_TRY {
		int n = lua_gettop(L);
		if (n < 4) {
			lua_pushstring(L, "display_message: requires at least 4 arguments: area, message, time and global");
			lua_error(L);
			return 0;
		}

		const char *area = lua_tostring(L, 1);
		if (area == NULL) {
			lua_pushstring(L, "display_message: first argument must be string");
			lua_error(L);
			return 0;		
		}

		const char *message = lua_tostring(L, 2);
		if (message == NULL) {
			lua_pushstring(L, "display_message: second argument must be string");
			lua_error(L);
			return 0;		
		}
		lua_Number time = lua_tonumber(L, 3);
		bool global = lua_toboolean(L, 4) != 0;
		GameMonitor->displayMessage(area, message, (float)time, global);
	} LUA_CATCH("lua_hooks_set_timer")
	return 0;		
}

static int lua_hooks_damage_map(lua_State *L) {
	LUA_TRY {
		int n = lua_gettop(L);
		if (n < 3) {
			lua_pushstring(L, "damage map: requires at least 3 arguments: x, y and hp");
			lua_error(L);
			return 0;
		}
		float x = (float)lua_tonumber(L, 1);
		float y = (float)lua_tonumber(L, 2);
		int hp = lua_tointeger(L, 3);
		float r = 0;
		if (n > 3) 
			r = (float)lua_tonumber(L, 4);
		
		if (r > 0) 
			Map->damage(v2<float>(x, y), hp, r);
		else 
			Map->damage(v2<float>(x, y), hp);
	} LUA_CATCH("damage_map")
	return 0;
}

static int lua_hooks_enable_ai(lua_State *L) {
	LUA_TRY {
		int n = lua_gettop(L);
		if (n < 1) {
			lua_pushstring(L, "enable_ai: requires classname");
			lua_error(L);
			return 0;
		}
		const char *classname = lua_tostring(L, 1);
		if (classname == NULL) {
			lua_pushstring(L, "enable_ai: first argument must be string");
			lua_error(L);
			return 0;		
		}
		GameMonitor->disable(classname, false);
	} LUA_CATCH("enable_ai")
	return 0;
}

static int lua_hooks_disable_ai(lua_State *L) {
	LUA_TRY {
		int n = lua_gettop(L);
		if (n < 1) {
			lua_pushstring(L, "disable_ai: requires classname");
			lua_error(L);
			return 0;
		}
		const char *classname = lua_tostring(L, 1);
		if (classname == NULL) {
			lua_pushstring(L, "disable_ai: first argument must be string");
			lua_error(L);
			return 0;		
		}
		GameMonitor->disable(classname, true);
	} LUA_CATCH("disable_ai")
	return 0;
}

static int lua_hooks_visual_effect(lua_State *L) {
	LUA_TRY {
		int n = lua_gettop(L);
		if (n < 2) {
			lua_pushstring(L, "visual_effect: requires name and duration");
			lua_error(L);
			return 0;
		}
		const char *name = lua_tostring(L, 1);
		if (name == NULL) {
			lua_pushstring(L, "visual_effect: first argument must be a string");
			lua_error(L);
			return 0;
		}
		float d = (float)lua_tonumber(L, 2);
		std::string effect = name;

		if (effect == "shaking") {
			int i = (n >= 3)?lua_tointeger(L, 3) : 4;
			
			Game->shake(d, i);
		} else throw_ex(("unknown visual effect name: %s", name));
	} LUA_CATCH("visual_effect")
	return 0;
}

static int lua_hooks_add_effect(lua_State *L) {
LUA_TRY {
	int n = lua_gettop(L);
	if (n < 3) {
		lua_pushstring(L, "add_effect requires object id, effect name and duration");
		lua_error(L);
		return 0;
	}

	int id = lua_tointeger(L, 1);
	Object *o = World->getObjectByID(id);

	if (o == NULL) {
		return 0;
	}

	std::string effect = lua_tostring(L, 2);
	float duration = lua_tonumber(L, 3);
	
	o->addEffect(effect, duration);
	return 0;	
} LUA_CATCH("lua_hooks_add_effect")	
}

static int lua_hooks_remove_effect(lua_State *L) {
LUA_TRY {
	int n = lua_gettop(L);
	if (n < 2) {
		lua_pushstring(L, "add_effect requires object id and effect name.");
		lua_error(L);
		return 0;
	}

	int id = lua_tointeger(L, 1);
	Object *o = World->getObjectByID(id);

	if (o == NULL) {
		return 0;
	}

	std::string effect = lua_tostring(L, 2);
	
	o->removeEffect(effect);
	return 0;	
} LUA_CATCH("lua_hooks_remove_effect")	
}

static int lua_hooks_add_waypoint(lua_State *L) {
LUA_TRY {
	int n = lua_gettop(L);
	if (n < 2) {
		lua_pushstring(L, "set_waypoint requires source object id and destination object id");
		lua_error(L);
		return 0;
	}
	int src_id = lua_tointeger(L, 1);
	int dst_id = lua_tointeger(L, 2);
	Object *src = World->getObjectByID(src_id);
	const Object *dst = World->getObjectByID(dst_id);
	if (src == NULL || dst == NULL) {
		if (src == NULL)
			LOG_ERROR(("object %d does not exists", src_id));
		if (dst == NULL)
			LOG_ERROR(("object %d does not exists", dst_id));
		return 0;
	}

	v2<int> dst_pos; 
	dst->getCenterPosition(dst_pos);

	Way way;
	way.push_back(dst_pos);

	src->setWay(way);
	return 0;
} LUA_CATCH("lua_hooks_add_waypoint")
}

static int lua_hooks_set_config_override(lua_State *L) {
LUA_TRY {
	int n = lua_gettop(L);
	if (n < 2) {
		lua_pushstring(L, "set_config_override requires key name and override value");
		lua_error(L);
		return 0;
	}
	const char * name = lua_tostring(L, 1);
	const char *value = lua_tostring(L, 2);
	if (name == NULL || value == NULL) {
		lua_pushstring(L, mrt::formatString("set_config_override: %s argument must be a string", (name == NULL)?"first":"second").c_str());
		lua_error(L);
		return 0;
	}
	
	Var var;
	var.fromString(value);
	Config->setOverride(name, var);
	Config->invalidateCachedValues();
	
} LUA_CATCH("lua_hooks_set_config_override")	
	return 0;
}

static int lua_hooks_play_sound(lua_State *L) {
LUA_TRY {
	int n = lua_gettop(L);
	if (n < 2) {
		lua_pushstring(L, "play_sound requires object_id(0 == listener), sound and optionally loop flag and gain level. ");
		lua_error(L);
		return 0;
	}
	int object_id = lua_tointeger(L, 1);
	const Object *o = NULL;
	if (object_id > 0) {
		o = World->getObjectByID(object_id);
		if (o == NULL)
			throw_ex(("object with id %d not found", object_id));
	} 
	
	const char * name = lua_tostring(L, 2);
	if (name == NULL) {
		lua_pushstring(L, "play_sound: second argument(sound name) must be a string");
		lua_error(L);
		return 0;
	}
	
	bool loop = false;
	float gain = 1.0f;
	if (n >= 3) 
		loop = lua_toboolean(L, 3);
	if (n >= 4) 
		loop = lua_tonumber(L, 4);
	Mixer->playSample(o, name, loop, gain);
	
} LUA_CATCH("lua_hooks_play_sound")	
	return 0;
}

static int lua_hooks_stop_sound(lua_State *L) {
LUA_TRY {
	int n = lua_gettop(L);
	if (n < 1) {
		lua_pushstring(L, "stop_sound requires object_id(0 == listener) and sound. ");
		lua_error(L);
		return 0;
	}
	int object_id = lua_tointeger(L, 1);
	const Object *o = NULL;
	if (object_id > 0) {
		o = World->getObjectByID(object_id);
		if (o == NULL)
			throw_ex(("object with id %d not found", object_id));
	} 
	
	const char * name = NULL;
	if (n >= 2) {
		name = lua_tostring(L, 2);
		if (name == NULL) {
			lua_pushstring(L, "stop_sound: second argument(sound name) must be a string");
			lua_error(L);
			return 0;
		}
	}
	if (name == NULL)
		Mixer->cancelAll(o);
	else 
		Mixer->cancelSample(o, name);
	
} LUA_CATCH("lua_hooks_stop_sound")	
	return 0;
}


static int lua_hooks_random(lua_State *L) {
LUA_TRY {
	int n = lua_gettop(L);
	if (n < 1) {
		lua_pushstring(L, "random requires upper limit value");
		lua_error(L);
		return 0;
	}
	n = lua_tointeger(L, 1);
	lua_pushinteger(L, mrt::random(n));
	return 1;
} LUA_CATCH("lua_random")
}


static int lua_hooks_players_number(lua_State *L) {
	lua_pushinteger(L, (int)PlayerManager->getSlotsCount());
	return 1;
}

static int lua_hooks_set_specials(lua_State *L) {
LUA_TRY {
	int n = lua_gettop(L);
	if (n < 1 || !lua_istable(L, 1)) {
		lua_pushstring(L, "set_specials requires table as first argument");
		lua_error(L);
		return 0;
	}
	std::vector<int> specials;
	lua_pushnil(L);  /* first key */
    while (lua_next(L, 1) != 0) {
		/* `key' is at index -2 and `value' at index -1 */
		//printf("%s - %s\n", lua_typename(L, lua_type(L, -2)), lua_typename(L, lua_type(L, -1)));
		int id = lua_tointeger(L, -1);
		specials.push_back(id);
		lua_pop(L, 1);  /* removes `value'; keeps `key' for next iteration */
	}
	GameMonitor->setSpecials(specials);
	return 0;
} LUA_CATCH("lua_random")
}

#include "finder.h"

void LuaHooks::load(const std::string &name) {
	LOG_DEBUG(("loading lua code from %s...", name.c_str()));

	mrt::Chunk data;
	Finder->load(data, name, false);
	std::string::size_type p = name.find('/');
	state.load(p != std::string::npos? name.substr(p + 1): name, data);

	lua_register(state, "print", lua_hooks_print);
	lua_register(state, "spawn", lua_hooks_spawn);
	lua_register(state, "object_exists", lua_hooks_object_exists);
	lua_register(state, "item_exists", lua_hooks_item_exists);
	lua_register(state, "show_item", lua_hooks_show_item);
	lua_register(state, "hide_item", lua_hooks_hide_item);
	lua_register(state, "kill_item", lua_hooks_kill_item);
	lua_register(state, "game_over", lua_hooks_game_over);
	lua_register(state, "display_message", lua_hooks_display_message);
	lua_register(state, "set_timer", lua_hooks_set_timer);
	lua_register(state, "reset_timer", lua_hooks_reset_timer);
	lua_register(state, "damage_map", lua_hooks_damage_map);
	lua_register(state, "enable_ai", lua_hooks_enable_ai);
	lua_register(state, "disable_ai", lua_hooks_disable_ai);
	lua_register(state, "play_tune", lua_hooks_play_tune);
	lua_register(state, "reset_tune", lua_hooks_reset_tune);
	lua_register(state, "object_property", lua_hooks_object_property);
	lua_register(state, "kill_object", lua_hooks_kill_object);
	lua_register(state, "set_slot_property", lua_hooks_set_slot_property);
	lua_register(state, "slot_property", lua_hooks_slot_property);
	lua_register(state, "load_map", lua_hooks_load_map);
	lua_register(state, "visual_effect", lua_hooks_visual_effect);
	lua_register(state, "add_effect", lua_hooks_add_effect);
	lua_register(state, "remove_effect", lua_hooks_remove_effect);
	lua_register(state, "add_waypoint", lua_hooks_add_waypoint);
	lua_register(state, "players_number", lua_hooks_players_number);
	lua_register(state, "set_config_override", lua_hooks_set_config_override);
	lua_register(state, "random", lua_hooks_random);
	lua_register(state, "spawn_random", lua_hooks_spawn_random);
	lua_register(state, "map_size", lua_hooks_map_size);
	lua_register(state, "set_specials", lua_hooks_set_specials);
	lua_register(state, "play_sound", lua_hooks_play_sound);
	lua_register(state, "stop_sound", lua_hooks_stop_sound);
	
	state.call(0, LUA_MULTRET);
	
	has_on_tick = check_function("on_tick");
	has_on_spawn = check_function("on_spawn");
	has_on_load = check_function("on_load");
}

bool LuaHooks::check_function(const std::string &name) {
	lua_settop(state, 0);
	
	lua_getglobal(state, name.c_str());
	bool r = !(lua_isnoneornil(state, -1));
	
	LOG_DEBUG(("checking for function: %s: %c", name.c_str(), r?'+':'-'));
	lua_pop(state, 1);

	return r;
}

const bool LuaHooks::on_spawn(const std::string &classname, const std::string &animation, const std::string &property) {
	if (!has_on_spawn)
		return true;
	
	lua_settop(state, 0);
	
	lua_getglobal(state, "on_spawn");
	lua_pushstring(state, classname.c_str());
	lua_pushstring(state, animation.c_str());
	lua_pushstring(state, property.c_str());

	state.call(3, 1);
	bool r = lua_toboolean(state, 1) != 0;
	lua_pop(state, 1);
	LOG_DEBUG(("on spawn returns %s", r?"true":"false"));

	return r;
}

void LuaHooks::on_load() {
	if (!has_on_load)
		return;

	lua_settop(state, 0);
	
	LOG_DEBUG(("calling on_load()"));
	lua_getglobal(state, "on_load");
	state.call(0, 0);
}


void LuaHooks::on_tick(const float dt) {
	if (!has_on_tick)
		return;
	
	lua_settop(state, 0);
	
	lua_getglobal(state, "on_tick");
	lua_pushnumber(state, dt);

	state.call(1, 0);
}

void LuaHooks::call(const std::string &method) {
	LOG_DEBUG(("calling %s()", method.c_str()));
	lua_settop(state, 0);

	lua_getglobal(state, method.c_str());
	state.call(0, 0);
}

void LuaHooks::call1(const std::string &method, const int id) {
	LOG_DEBUG(("calling %s(%d)", method.c_str(), id));
	lua_settop(state, 0);

	lua_getglobal(state, method.c_str());
	lua_pushinteger(state, id);
	
	state.call(1, 0);
}

void LuaHooks::clear() {
	state.clear();
	has_on_tick = has_on_spawn = has_on_load = false;
}

LuaHooks::LuaHooks() : has_on_tick(false), has_on_spawn(false), has_on_load(false) {}
