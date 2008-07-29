#ifndef BTANKS_MENU_HOSTITEM_H__
#define BTANKS_MENU_HOSTITEM_H__

#include "container.h"
#include "mrt/sys_socket.h"
#include "game_type.h"

class Label;
namespace sdlx { class Font; }

class HostItem : public Container {
public:
	mrt::Socket::addr addr;
	std::string name, map;
	int ping, players, slots;
	GameType game_type;
	
	HostItem();
	void update();
	void start(float t);
	void tick(const float dt);
	virtual void render(sdlx::Surface &surface, const int x, const int y) const;
private: 
	Label * _line;
	const sdlx::Font *_font;
	float timer;
};

#endif
