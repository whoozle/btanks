#ifndef BTANKS_MENU_HOSTITEM_H__
#define BTANKS_MENU_HOSTITEM_H__

#include "container.h"
#include "mrt/sys_socket.h"
#include "game_type.h"

class Label;

class HostItem : public Container {
public:
	mrt::Socket::addr addr;
	std::string name, map;
	int ping, players, slots;
	GameType game_type;
	
	HostItem();
	void update();
private: 
	Label * _line;
};

#endif
