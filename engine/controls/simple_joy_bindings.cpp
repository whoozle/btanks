#include "simple_joy_bindings.h"
#include "mrt/exception.h"
#include "config.h"
#include <stdlib.h>
#include "sdlx/sdlx.h"
#include "sdlx/joystick.h"
#include "mrt/logger.h"
#include <set>
#include "config.h"
#include "player_state.h"

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
			type = Axis;
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
			type = Button;
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

			type = Hat;
			index = i;
			value = j;
			need_save = true;
			return;
		}
	
	}
	throw_ex(("invalid control type '%c'", t));
}


const SimpleJoyBindings::State & SimpleJoyBindings::get(int idx) const {
	if (idx < 0 || idx >= 8)
		throw_ex(("invalid state index %d", idx));
	return state[idx];
}

void SimpleJoyBindings::set(int idx, const State &s) {
	if (idx < 0 || idx >= 8)
		throw_ex(("invalid state index %d", idx));
	
	if (state[idx] == s)
		return;
	
	LOG_DEBUG(("setting %d to %s", idx, s.get_name().c_str()));
	
	for(int i = 0; i < 8; ++i) {
		if (i != idx && state[i] == s)
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
	config_base("player.controls.joystick." + profile + ".") { 
	LOG_DEBUG(("loading joystick bindings for the '%s'", profile.c_str()));
	
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
	Config->get(config_base + "dead-zone", dead_zone, 0.8f);

	for(int i = 0; i < 8; ++i) {
		std::string key = config_base + names[i];
		if (Config->has(key)) {
			LOG_DEBUG(("found config key %s", key.c_str()));
			std::string value; 
			Config->get(key, value, std::string());
			try {
				state[i].from_string(value);
				LOG_DEBUG(("loaded %d -> %s", i, state[i].to_string().c_str()));
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
			dst.need_save |= src.need_save;
			break;
		case State::Hat:
			dst.type = src.type;
			dst.index = src.index;
			if (src.value & (SDL_HAT_UP | SDL_HAT_DOWN))
				dst.value = src.value ^ (SDL_HAT_UP | SDL_HAT_DOWN);
			if (src.value & (SDL_HAT_LEFT | SDL_HAT_RIGHT))
				dst.value = src.value ^ (SDL_HAT_LEFT | SDL_HAT_RIGHT);
			dst.need_save |= src.need_save;
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
	//LOG_DEBUG(("validate"));
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
	case Button: {
		if (index < 0)
			throw_ex(("invalid button index %d", index));
		if (index > 10) 
			return mrt::format_string("(%d)", index + 1);
			
		std::string r = "\342\221";
		r += (char)(0xa0 + index);
		return r;
	}
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

//"left", "right", "up", "down", "fire", "alt-fire", "disembark", "hint-ctrl"

void SimpleJoyBindings::update(PlayerState &dst, const sdlx::Joystick &joy) const {
	if (!joy.opened())
		throw_ex(("joystick was not opened"));
	
	for(int i = 0; i < 8; ++i) {
		int vi = 0;
		const State &s = state[i];
		switch(s.type) {

			case State::Button: 
				vi = joy.get_button(s.index)? 1:0;
			break;

			case State::Axis: 
				vi = (joy.get_axis(s.index) * s.value >= (int)(dead_zone * 32767))? 1:0;
			break;

			case State::Hat: 
				vi = ((joy.get_hat(s.index) & s.value) == s.value)? 1:0;
			break;
			
			case State::None: break;
		}
		switch(i) {
		case 0: dst.left = vi; break;
		case 1: dst.right = vi; break;
		case 2: dst.up = vi; break;
		case 3: dst.down = vi; break;
		case 4: dst.fire = vi; break;
		case 5: dst.alt_fire = vi; break;
		case 6: dst.leave = vi; break;
		case 7: dst.hint_control = vi; break;
		}
	}
}

void SimpleJoyBindings::set_dead_zone(const float dz) {
	dead_zone = dz;
	Config->set(config_base + "dead-zone", dz);
}
