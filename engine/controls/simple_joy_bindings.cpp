#include "simple_joy_bindings.h"
#include "mrt/exception.h"
#include "config.h"
#include <stdlib.h>
#include "sdlx/sdlx.h"
#include "sdlx/joystick.h"
#include <set>

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
			need_save = true;
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
			need_save = true;
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
			need_save = true;
			return;
		}
	
	}
	throw_ex(("invalid control type '%c'", t));
}

void SimpleJoyBindings::set(int idx, const State &s) {
	if (idx < 0 || idx >= 8)
		throw_ex(("invalid state index %d", idx));
	LOG_DEBUG(("setting %d to %s", idx, s.get_name().c_str()));
	
	for(int i = 0; i < 8; ++i) {
		if (state[i] == s)
			state[i].clear();
	}
	
	state[idx] = s;
	state[idx].need_save = true;
	switch(idx) {
		case 0: 
			set_opposite(state[1], state[0]); break;
		case 1: 
			set_opposite(state[0], state[1]); break;
		case 2: 
			set_opposite(state[3], state[2]); break;
		case 3: 
			set_opposite(state[2], state[3]); break;
	}
	validate();
}

SimpleJoyBindings::SimpleJoyBindings(const std::string &profile, const sdlx::Joystick &joy) : 
	config_base("player.controls." + profile + ".") { 
	
	axis = joy.get_axis_num();
	buttons = joy.get_buttons_num();
	hats = joy.get_hats_num();
	
	reload();
}

static const char * names[] = {"left", "right", "up", "down", "fire", "alt-fire", "disembark", "hint-ctrl"};

void SimpleJoyBindings::save() {
	for(int i = 0; i < 8; ++i) {
		if (state[i].need_save) {
			Config->set(config_base + names[i], state[i].to_string());
		}
	}	
}

void SimpleJoyBindings::reload() {
	for(int i = 0; i < 8; ++i) {
		if (Config->has(config_base + names[i])) {
			std::string value; 
			Config->get(config_base + names[i], value, std::string());
			try {
				state[i].from_string(value);
			} CATCH("reload", continue);
		} else {
			state[i].clear();
		}
	}
	validate();
}

void SimpleJoyBindings::set_opposite(State &dst, const State &src) {
	switch(src.type) {
		case State::Axis:
			dst.type = src.type;
			dst.value = -src.value;
			dst.index = src.index;
			break;
		case State::Hat:
			dst.type = src.type;
			dst.index = src.index;
			dst.value = ((~src.value) & (SDL_HAT_UP | SDL_HAT_DOWN | SDL_HAT_LEFT | SDL_HAT_RIGHT));
			break;
		default: 
			break;
	}
}

bool SimpleJoyBindings::valid() const {
	std::set<State> used_controls;
	
	for(int i = 0; i < 8; ++i) {
		if (state[i].type != State::None) {
			used_controls.insert(state[i]);
		}
	}
	return used_controls.size() == 8;
}

void SimpleJoyBindings::validate() {
	std::set<State> used_controls;
	
	for(int i = 0; i < 8; ++i) {
		if (state[i].type != State::None) {
			used_controls.insert(state[i]);
		}
	}
	if (used_controls.size() == 8)
		return;
	
	{
		//eliminated duplicates
		std::set<State> seen(used_controls);
		for(int i = 0; i < 8; ++i) {
			if (state[i].type == State::None) 
				continue;
			if (seen.find(state[i]) == seen.end()) {
				//duplicate
				state[i].clear();
			} else {
				seen.erase(state[i]);
			}
		}
	}
	
	for(int idx = 0; idx < 4; idx += 2) {
		if (state[idx].type != State::None) 
			continue;
		
		State s;
		s.type = State::Axis;
		s.value = -1;

		for(int i = 0; i < axis; ++i) {
			s.index = i;
			if (used_controls.find(s) == used_controls.end()) {
				state[idx] = s;
				used_controls.insert(s);
				set_opposite(state[idx + 1], state[idx]);
				goto found;
			}
		}
		s.type = State::Hat;
		s.value = idx == 0 ?SDL_HAT_LEFT: SDL_HAT_UP;
		for(int i = 0; i < hats; ++i) {
			s.index = i;
			if (used_controls.find(s) == used_controls.end()) {
				state[idx] = s;
				used_controls.insert(s);
				set_opposite(state[idx + 1], state[idx]);
				goto found;
			}
		}
		found:;
	}

	for(int i = 0; i < 8; ++i) {
		if (state[i].type != State::None) 
			continue;
		State s;
		s.type = State::Button;
		
		for(int b = 0; b < buttons; ++b) {
			s.index = b;
			if (used_controls.find(s) == used_controls.end()) {
				used_controls.insert(s);
				state[i] = s;
				break;
			}
		}
	}
}

const std::string SimpleJoyBindings::get_name(int idx) const {
	if (idx < 0 || idx >= 8) 
		throw_ex(("invalid control index %d", idx));
	return state[idx].get_name();
}

const std::string SimpleJoyBindings::State::get_name() const {
	switch(type) {
	case Button:
		return mrt::format_string("(%d)", index + 1);
	case Axis:
		return mrt::format_string("Axis %d %c", index + 1, value > 0? '+': '-');
	case Hat: {
			std::string ctrls;
			std::vector<std::string> c;
			if (value & SDL_HAT_LEFT)
				c.push_back("left");
			if (value & SDL_HAT_RIGHT)
				c.push_back("right");
			if (value & SDL_HAT_UP)
				c.push_back("up");
			if (value & SDL_HAT_DOWN)
				c.push_back("down");
			mrt::join(ctrls, c, "+");
			return mrt::format_string("Hat %d %s", index + 1, ctrls.c_str());
		}
	default: 
		return std::string();
	}
}
