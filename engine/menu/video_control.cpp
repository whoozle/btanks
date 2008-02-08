#include "video_control.h"
#include "resource_manager.h"
#include "finder.h"
#include "sdlx/sdl_ex.h"
#include "sdlx/surface.h"
#include "mrt/chunk.h"

#define SMPEG_CHECK(f) do { const char * err; if ((err = SMPEG_error(mpeg)) != NULL) throw_ex(("%s: %s", f, err)); } while(0)

std::map<const SDL_Surface *, VideoControl *> video_controls;

static void update(SDL_Surface* dst, int x, int y, unsigned int w, unsigned int h) {
	///LOG_DEBUG(("%p: update %d, %d, %d, %d", (void *)dst, x, y, w, h));
	VideoControl *vc = video_controls[dst];
	if (vc == NULL)
		return;
	vc->copy(x, y, w, h);
}

void VideoControl::copy(const int x, const int y, const int w, const int h) {
	updated = true;
}


VideoControl::VideoControl(const std::string &base, const std::string &name) : 
base(base), name(name), mpeg(0), lock(SDL_CreateMutex()), active(false), updated(false)  {
	if (lock == NULL)
		throw_sdl(("SDL_CreateMutex"));
	
	std::string fname = "maps/" + name + ".jpg";
	if (Finder->exists(base, fname)) {
		screenshot = ResourceManager->loadSurface("../" + fname);
	} else 
		screenshot = ResourceManager->loadSurface("../maps/null_video.png");
	
	fname = "maps/" + name + ".mpg";
	if (Finder->exists(base, fname)) {
		{
			mrt::Chunk video_data;
			Finder->load(video_data, fname);
			LOG_DEBUG(("video file loaded (%u bytes)", (unsigned)video_data.getSize()));

			mpeg = SMPEG_new_data(video_data.getPtr(), video_data.getSize(), &mpeg_info, 0);
			if (mpeg == NULL)
				throw_sdl(("SMPEG_new_data: %s", SDL_GetError()));
		}

		shadow.createRGB(screenshot->getWidth(), screenshot->getHeight(), 24, SDL_SWSURFACE);
		shadow.fill(0);
		shadow.convertAlpha();
		shadow.setAlpha(0, 0);

		frame.createRGB(screenshot->getWidth(), screenshot->getHeight(), 24, SDL_SWSURFACE);
		frame.fill(0);
		frame.convertAlpha();
		frame.setAlpha(0, 0);

		LOG_DEBUG(("video file info: %dx%d, %.02g seconds", mpeg_info.width, mpeg_info.height, mpeg_info.total_time));
		SMPEG_enableaudio(mpeg, 0);
		SMPEG_CHECK("SMPEG_enableaudio");
		SMPEG_enablevideo(mpeg, 1);
		SMPEG_CHECK("SMPEG_enablevideo");
		
		SDL_Surface *dst = shadow.getSDLSurface();
		video_controls[dst] = this;
		SMPEG_setdisplay(mpeg, dst, lock, update);
		SMPEG_CHECK("SMPEG_setdisplay");
		
		SMPEG_scaleXY(mpeg, screenshot->getWidth(), screenshot->getHeight());
		SMPEG_CHECK("SMPEG_scaleXY");
		
		SMPEG_loop(mpeg, 1);
		SMPEG_CHECK("SMPEG_loop");
		checkStatus();
		//SMPEG_play(mpeg);
		//SMPEG_CHECK("SMPEG_play");
	}
}

void VideoControl::activate(const bool a) {
	active = a;
}

void VideoControl::checkStatus() {
	if (mpeg == NULL)
		return;
	
	switch(SMPEG_status(mpeg)) {
	case SMPEG_PLAYING: 
		if (!active) {
			LOG_DEBUG(("calling SMPEG_pause"));
			SMPEG_pause(mpeg);
		}
		break;
	case SMPEG_STOPPED: 
		if (active) {
			LOG_DEBUG(("calling SMPEG_pause"));
			SMPEG_pause(mpeg);
		}
		break;
	case SMPEG_ERROR: 
		LOG_DEBUG(("SMPEG error: %s", SMPEG_error(mpeg)));
		SMPEG_delete(mpeg);
		mpeg = NULL;
		break;
	}
}

void VideoControl::tick(const float dt) {
	Control::tick(dt);
	if (mpeg == NULL) 
		return;
		
	checkStatus();

	if (updated) {
		//LOG_DEBUG(("syncing frame with shadow"));
		frame.createRGB(mpeg_info.width, mpeg_info.height, 24, SDL_SWSURFACE);
		frame.fill(0);
		frame.convertAlpha();

		SDL_mutexP(lock);
		try {
			shadow.setAlpha(0, 0);
			frame.copyFrom(shadow, 0, 0);
			frame.setAlpha(0, 0);
		} catch(...) {
			SDL_mutexV(lock);
			throw;
		}
		updated = false;
		SDL_mutexV(lock);

		//frame.convertAlpha();
	}
}

void VideoControl::render(sdlx::Surface &surface, const int x, const int y) {
	if (mpeg == NULL || !active) {
		surface.copyFrom(*screenshot, x, y);
		return;
	}
	//int dx = (screenshot->getWidth() - mpeg_info.width) / 2;
	//int dy = (screenshot->getHeight() - mpeg_info.height) / 2;
	int dx = 0, dy = 0;
	//LOG_DEBUG(("render %d %d", dx, dy));
	
	surface.copyFrom(frame, x + dx, y + dy);
}

void VideoControl::getSize(int &w, int &h) const {
	w = screenshot->getWidth();
	h = screenshot->getHeight();
}

VideoControl::~VideoControl() {
	video_controls.erase(shadow.getSDLSurface());
	if (mpeg != NULL) {
		SMPEG_stop(mpeg);
		SMPEG_delete(mpeg);
	}
	SDL_DestroyMutex(lock);
}
