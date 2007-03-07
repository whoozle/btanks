#ifndef BTANKS_CONTROL_PICKER_H__
#define BTANKS_CONTROL_PICKER_H__

#include "container.h"
#include <string>

class ControlPicker : public Container {
public: 
	ControlPicker(const int w, const int h, const std::string &font, const std::string &label, const std::string &config_key);
	void save(); 
	void reload();
private: 
	std::string _config_key;
};

#endif

