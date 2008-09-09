
/* Battle Tanks Game
 * Copyright (C) 2006-2008 Battle Tanks team
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

/* 
 * Additional rights can be granted beyond the GNU General Public License 
 * on the terms provided in the Exception. If you modify this file, 
 * you may extend this exception to your version of the file, 
 * but you are not obligated to do so. If you do not wish to provide this
 * exception without modification, you must delete this exception statement
 * from your version and license this file solely under the GPL without exception. 
*/
#include "control_picker.h"
#include "label.h"
#include "chooser.h"
#include "resource_manager.h"
#include "config.h"
#include "sdlx/joystick.h"

ControlPicker::ControlPicker(const int w, const std::string &font, const std::string &label, const std::string &config_key, const std::string &def, const std::string &variant) 
: _config_key(config_key), _default(def) {
	int bw, bh;
	
	Label *l = new Label(font, label);
	l->get_size(bw, bh);
	add(0, 0, l);

	_values.push_back("mouse");
	
	if (variant == "split") {
		_values.push_back("keys-1");
		_values.push_back("keys-2");
	} else {
		_values.push_back("keys");
	}
	

	int base = _values.size();
	
	int n = sdlx::Joystick::getCount();
	for(int i = 0; i < 4; ++i) {
		_values.push_back(mrt::format_string("joy-%d", i + 1));
	}

	_controls = new Chooser("medium", _values, variant == "split"?"menu/controls_split.png":"menu/controls.png");

	for(int i = 0; i < (int)_values.size(); ++i) {
		if (i >= base + n) 
			_controls->disable(i);
	}

	int cw, ch;
	_controls->get_size(cw, ch);
	add(w - 100 - cw/2, 0, _controls);
	
	reload();
}

void ControlPicker::save() {
	Config->set(_config_key, _controls->getValue());
}

void ControlPicker::reload() {
	TRY {
		std::string cm;
		Config->get(_config_key, cm, _default);
		_controls->set(cm);
	} CATCH("reload", {})
}
