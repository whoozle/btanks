#include "control_picker.h"
#include "label.h"
#include "chooser.h"
#include "resource_manager.h"
#include "config.h"
#include "sdlx/joystick.h"

ControlPicker::ControlPicker(const int w, const std::string &font, const std::string &label, const std::string &config_key, const std::string &variant) 
: _config_key(config_key) {
	int bw, bh;
	
	Label *l = new Label(font, label);
	l->getSize(bw, bh);
	add(0, 0, l);
	
	if (variant == "split") {
		_values.push_back("keys-1");
		_values.push_back("keys-2");
	} else {
		_values.push_back("keys");
	}
	
	int n = sdlx::Joystick::getCount();
	for(int i = 0; i < n; ++i) {
		_values.push_back(mrt::formatString("joy-%d", i + 1));
	}

	int cw, ch;
	_controls = new Chooser("medium", _values);
	_controls->getSize(cw, ch);
	add(w - 100 - cw/2, 0, _controls);
	
	reload();
}

void ControlPicker::save() {
	Config->set(_config_key, _controls->getValue());
}

void ControlPicker::reload() {
	TRY {
		std::string cm;
		Config->get(_config_key, cm, "keys");
		_controls->set(cm);
	} CATCH("reload", {})
}
