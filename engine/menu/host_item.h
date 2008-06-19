#ifndef BTANKS_MENU_HOSTITEM_H__
#define BTANKS_MENU_HOSTITEM_H__

#include "container.h"
#include "mrt/sys_socket.h"

class Label;

class HostItem : public Container {
public:
	mrt::Socket::addr addr;
	std::string name, map;
	int ping, players, slots;
	
	HostItem();
	void update();
private: 
	Label * _line;
	int _default_port;
};

#endif
