#ifndef BTANKS_MENU_HOSTITEM_H__
#define BTANKS_MENU_HOSTITEM_H__

#include "container.h"

class Label;

class HostItem : public Container {
public:
	std::string name, ip;
	int ping, players, slots;
	
	HostItem();
	void update();
private: 
	Label * _line;
};

#endif
