#ifndef BTANKS_MENU_HOSTITEM_H__
#define BTANKS_MENU_HOSTITEM_H__

#include "container.h"

class Label;

class HostItem : public Container {
public:
	std::string host, ip;
	const int ping;
	const int players, slots;
	
	HostItem();
	void update();
private: 
	Label * _line;
};

#endif
