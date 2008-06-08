#ifndef BT_ENGINE_VIDEO_CONTROL_DISABLED_H__
#define BT_ENGINE_VIDEO_CONTROL_DISABLED_H__

#include <string>
#include "control.h"
#include <smpeg/smpeg.h>
#include "sdlx/surface.h"

class DisabledVideoControl : public Control {
public: 
	DisabledVideoControl(const std::string &base, const std::string &name);
	virtual void render(sdlx::Surface &surface, const int x, const int y) const;
	virtual void get_size(int &w, int &h) const;

private: 
	const sdlx::Surface *screenshot;
};


#endif

