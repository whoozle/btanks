#include "simple_joy_bindings.h"
#include "mrt/exception.h"
#include "config.h"
#include <stdlib.h>

const std::string SimpleJoyBindings::State::to_string() const {
	switch(type) {
		case None: return std::string();
		case Axis: return mrt::format_string("a%c%d", value > 0?'+':'-', index);
		case Button: return mrt::format_string("b%d", index);
		case Hat: return mrt::format_string("h%d %d", index, value);
	}
	throw_ex(("invalid type value %d", (int)type));
}

void SimpleJoyBindings::State::from_string(const std::string &str) {
	if (str.empty())
		throw_ex(("value for control must not be empty"));
	char t = str[0];
	switch(t) {
		case 'a': {
			if (str.size() < 3)
				throw_ex(("invalid control string '%s'", str.c_str()));
			char v = str[1];
			if (v != '+' && v != '-')
				throw_ex(("invalid axis direction '%c'", v));
			int i = atoi(str.c_str() + 2);
			if (i < 0) 
				throw_ex(("invalid axis index (%d)", i));
			index = i;
			value = v == '+'? 1: -1;
			return;
		}

		case 'b': {
			if (str.size() < 2)
				throw_ex(("invalid control string '%s'", str.c_str()));
			int i = atoi(str.c_str() + 1);
			if (i < 0) 
				throw_ex(("invalid button index (%d)", i));
			index = i;
			value = 0;
			return;
		}

		case 'h': {
			if (str.size() < 2)
				throw_ex(("invalid control string '%s'", str.c_str()));
			std::string::size_type pos = str.rfind(' ');
			if (pos == str.npos)
				throw_ex(("invalid control string '%s'", str.c_str()));
			
			int i = atoi(str.c_str() + 1);
			if (i < 0) 
				throw_ex(("invalid hat index (%d)", i));

			int j = atoi(str.c_str() + pos);
			if (j < 0) 
				throw_ex(("invalid hat value (%d)", j));

			index = i;
			value = j;
			return;
		}
	
	}
	throw_ex(("invalid control type '%c'", t));
}


SimpleJoyBindings::SimpleJoyBindings(const std::string &profile, int axis, int buttons, int hats): 
config_base("player.controls." + profile + "."), axis(axis), buttons(buttons), hats(hats) {
	reload();
}

void SimpleJoyBindings::save() {

}

void SimpleJoyBindings::reload() {
	const char * names[] = {"up", "down", "left", "right", "fire", "alt-fire", "disembark", "hint-ctrl"};
	for(int i = 0; i < 8; ++i) {
		if (Config->has(config_base + names[i])) {
			std::string value; 
			Config->get(config_base + names[i], value, std::string());
			try {
				state[i].from_string(value);
			} CATCH("reload", continue);
		}
	}
}

