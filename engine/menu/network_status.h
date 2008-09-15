#ifndef BTANKSMENU_NETWORK_STATUS_H__
#define BTANKSMENU_NETWORK_STATUS_H__

#include "tooltip.h"

class NetworkStatusControl : public Tooltip {
public: 
	NetworkStatusControl();
	void render(sdlx::Surface &surface, const int x, const int y) const;
	virtual bool onMouse(const int button, const bool pressed, const int x, const int y);

private: 
	mutable const sdlx::Surface * _bclose;
	mutable sdlx::Rect _close_area;
};

#endif

