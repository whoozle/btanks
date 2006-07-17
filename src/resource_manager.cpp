#include "resource_manager.h"
#include "mrt/logger.h"
#include "sdlx/surface.h"
#include "animated_object.h"

IMPLEMENT_SINGLETON(ResourceManager, IResourceManager)


void IResourceManager::start(const std::string &name, Attrs &attr) {	
	if (name == "resources") {
		_tw = atol(attr["tile_width"].c_str());
		if (_tw == 0)
			throw_ex(("resources tag must contain `tile_width' attribute (default tile width)"));
		_th = atol(attr["tile_height"].c_str());
		if (_th == 0)
			throw_ex(("resources tag must contain `tile_height' attribute (default tile height)"));
		if (attr["version"].size() == 0)
			throw_ex(("resources tag must contain `version' attribute, now only 0.3 supported"));
		LOG_DEBUG(("file version: %s", attr["version"].c_str()));
	} else if (name == "animation") {
		const std::string &id = attr["id"];
		if (id.size() == 0)
			throw_ex(("animation.id was not set"));

		long tw = atol(attr["tile_width"].c_str());
		long th = atol(attr["tile_height"].c_str());
		if (tw == 0) tw = _tw;
		if (th == 0) th = _th;

		float speed = atof(attr["speed"].c_str());
		if (speed == 0) speed = 1;

		sdlx::Surface *s = NULL;
		TRY { 
			s = new sdlx::Surface;
			const std::string fname = "data/tiles/" + attr["tile"];
			s->loadImage(fname);
			LOG_DEBUG(("loaded animation '%s' from '%s'", id.c_str(), fname.c_str()));
			_animations[id] = new AnimatedObject(s, tw, th, speed);
		} CATCH("animation", { delete s; s = NULL; });
	} else if (name == "pose") {
		//nope
	} else LOG_WARN(("unhandled tag: %s", name.c_str()));
}

void IResourceManager::end(const std::string &name) {
	if (name == "pose") {
		LOG_DEBUG(("pose frames: %s", _data.c_str()));
		std::vector<std::string> frames;
		mrt::split(frames, _data, ",");
		for(size_t i = 0; i < frames.size(); ++i) {
			//LOG_DEBUG(("%d: %s", i, frames[i].c_str()));
			mrt::trim(frames[i]);
		}
	}
}
void IResourceManager::charData(const std::string &data) {
	_data = data;
}

IResourceManager::IResourceManager() {
	//LOG_DEBUG(("IResourceManager ctor"));
}

AnimatedObject *IResourceManager::getAnimation(const std::string &id) {
	AnimationMap::iterator i;
	if ((i = _animations.find(id)) == _animations.end()) 
		throw_ex(("could not find animation with id '%s'", id.c_str()));
	return i->second;
}


void IResourceManager::init(const std::string &fname) {
	LOG_DEBUG(("loading resources from file: %s", fname.c_str()));
	parseFile(fname);
}


void IResourceManager::clear() {
	LOG_DEBUG(("freeing resources"));
	for(AnimationMap::iterator i = _animations.begin(); i != _animations.end(); ++i) {
		delete i->second;
		i->second = NULL;
	}
	_animations.clear();
}

IResourceManager::~IResourceManager() {
	clear();
}

