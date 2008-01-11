#ifndef BTANKS_MENU_TEXTUAL_CONTROL_H__
#define BTANKS_MENU_TEXTUAL_CONTROL_H__

#include "control.h"

class BTANKSAPI TextualControl : public Control {
public: 
	virtual const std::string get() const = 0;
};

#endif

