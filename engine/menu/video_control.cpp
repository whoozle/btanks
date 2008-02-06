#include "video_control.h"
#include "resource_manager.h"
#include "finder.h"
#include "sdlx/sdl_ex.h"
#include "sdlx/surface.h"
#include "mrt/chunk.h"

#define SMPEG_CHECK(f) do { const char * err; if ((err = SMPEG_error(mpeg)) != NULL) throw_ex(("%s: %s", f, err)); } while(0)

VideoControl::VideoControl(const std::string &base, const std::string &name) : 
base(base), name(name), mpeg(0), active(false), attached(false) {
	std::string fname = "maps/" + name + ".jpg";
	if (Finder->exists(base, fname)) {
		screenshot = ResourceManager->loadSurface("../" + fname);
	} else 
		screenshot = ResourceManager->loadSurface("../maps/null_video.png");
	
	fname = "maps/" + name + ".mpg";
	if (Finder->exists(base, fname)) {
		mrt::Chunk video_data;
		Finder->load(video_data, fname);
		LOG_DEBUG(("video file loaded (%u bytes)", (unsigned)video_data.getSize()));

		mpeg = SMPEG_new_data(video_data.getPtr(), video_data.getSize(), &mpeg_info, 0);
		if (mpeg == NULL)
			throw_sdl(("SMPEG_new_data: %s", SDL_GetError()));

		video_data.free();
		//some stuff		
		LOG_DEBUG(("video file info: %dx%d, %.02g seconds", mpeg_info.width, mpeg_info.height, mpeg_info.total_time));
		active = true;
		SMPEG_enableaudio(mpeg, 0);
		SMPEG_CHECK("SMPEG_enableaudio");
		SMPEG_loop(mpeg, 1);
		SMPEG_CHECK("SMPEG_loop");
		SMPEG_play(mpeg);
		SMPEG_CHECK("SMPEG_play");
	}
}

void VideoControl::render(sdlx::Surface &surface, const int x, const int y) {
	if (mpeg == NULL || !active) {
		surface.copyFrom(*screenshot, x, y);
		return;
	}
	if (!attached) {
		SMPEG_setdisplay(mpeg, surface.getSDLSurface(), NULL, NULL);
		attached = true;
	}
	int dx = screenshot->getWidth() - mpeg_info.width;
	int dy = screenshot->getHeight() - mpeg_info.height;
	SMPEG_move(mpeg, x + dx, y + dy);
	SMPEG_CHECK("SMPEG_move");
	static int idx = 0;
	SMPEG_renderFrame(mpeg, idx++);
	SMPEG_CHECK("SMPEG_renderFrame");
}

void VideoControl::getSize(int &w, int &h) const {
	w = screenshot->getWidth();
	h = screenshot->getHeight();
}
