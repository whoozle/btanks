#ifndef BT_ENGINE_VIDEO_CONTROL_H__
#define BT_ENGINE_VIDEO_CONTROL_H__

#include <string>
#include "control.h"
#include "sdlx/surface.h"
#include <SDL_thread.h>
#ifdef ENABLE_SMPEG
#	include <smpeg/smpeg.h>
#endif

class VideoControl : public Control {
public: 
	VideoControl(const std::string &base, const std::string &name);
	virtual void render(sdlx::Surface &surface, const int x, const int y) const;
	virtual void get_size(int &w, int &h) const;
	virtual void tick(const float dt);
	~VideoControl();
	
	virtual void activate(const bool active);
private: 
	void checkStatus();
	std::string base, name;
	const sdlx::Surface *screenshot;
#ifdef ENABLE_SMPEG
	SMPEG * mpeg;
	SMPEG_Info mpeg_info;
#else
	void * mpeg;
#endif
	sdlx::Surface shadow, frame;
	SDL_mutex * lock;
	bool active, started;
	//volatile bool updated;
};


#endif

