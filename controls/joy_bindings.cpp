#include "joy_bindings.h"
#include "config.h"

void Bindings::clear() {
	_bindings.clear();
}

void Bindings::load(const std::string &profile, const int buttons, const int axes, const int hats) {
	_bindings.clear();
	_profile = profile;
	
	static const char* names[] = {"button", "axis", "hat"};
	static const JoyControlType types[] = {tButton, tAxis, tHat};

	const int nums[] = {buttons, axes, hats};

	for(int c = 0; c < 3; ++c) {
		for(int i = 0; i < nums[c]; ++i) {
			std::string name = mrt::formatString("player.controls.joystick.%s.%s.%d", profile.c_str(), names[c], i);
			int hard_id;
			if (Config->has(name)) {
				Config->get(name, hard_id, i);
				_bindings.insert(BaseBindings::value_type(BaseBindings::key_type(types[c], hard_id), i));
			}
		}
	}
	LOG_DEBUG(("loaded profile '%s' with %u bindings", _profile.c_str(), (unsigned)_bindings.size()));
}

void Bindings::save() {
	for(BaseBindings::const_iterator i = _bindings.begin(); i != _bindings.end(); ++i) {
		std::string name;
		switch(i->first.first) {
		case tButton: 
			name = "button";
			break;
		case tAxis: 
			name = "axis";
			break;
		case tHat: 
			name = "hat";
			break;
		}
		int hard_id = i->first.second;
		int virt_id = i->second;
		Config->set(mrt::formatString("player.controls.joystick.%s.%s.%d", _profile.c_str(), name.c_str(), virt_id), hard_id);
	}
}

void Bindings::set(const JoyControlType type, const int hard_id, const int virt_id) {
	if (hard_id == virt_id)
		return;
	
	_bindings.insert(BaseBindings::value_type(BaseBindings::key_type(type, hard_id), virt_id));
}

const bool Bindings::has(const JoyControlType type, const int hard_id) const {
	return _bindings.find(BaseBindings::key_type(type, hard_id)) != _bindings.end() ;
}

