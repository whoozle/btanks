#ifndef __MENU_UPPER_BOX_H__
#define __MENU_UPPER_BOX_H__

#include <string>
#include "box.h"

class UpperBox : public Box {
public: 
	std::string value;

	virtual void init(int w, int h, const bool server);
	virtual void render(sdlx::Surface &surface, const int x, const int y);
private: 
	bool _server;	
};

#endif
