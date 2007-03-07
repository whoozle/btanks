#include "control_picker.h"
#include "label.h"
#include "resource_manager.h"

ControlPicker::ControlPicker(const int w, const int h, const std::string &font, const std::string &label, const std::string &config_key) : _config_key(config_key) {
	int bw, bh;
	
	Label *l = new Label(ResourceManager->loadFont(font, false), label);
	l->getSize(bw, bh);
	add(0, 0, l);
}

void ControlPicker::save() {

}

void ControlPicker::reload() {

}
