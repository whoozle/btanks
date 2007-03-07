#ifndef BTANKS_CONTROL_PICKER_H__
#define BTANKS_CONTROL_PICKER_H__

#include "container.h"
#include <string>

class Chooser;
class ControlPicker : public Container {
public: 
	ControlPicker(const int w, const std::string &font, const std::string &label, const std::string &config_key, const std::string &variant);
	void save(); 
	void reload();
private: 
	std::string _config_key;
	std::vector<std::string> _values;
	Chooser *_controls;
};

#endif

