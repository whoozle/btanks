
/* Battle Tanks Game
 * Copyright (C) 2006-2007 Battle Tanks team
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

#include "resource_manager.h"
#include "mrt/logger.h"
#include "sdlx/surface.h"
#include "sdlx/font.h"
#include "sdlx/c_map.h"
#include "object.h"
#include "animation_model.h"
#include "utils.h"
#include "sound/mixer.h"
#include "config.h"
#include "finder.h"

#include <algorithm>

IMPLEMENT_SINGLETON(ResourceManager, IResourceManager);

void IResourceManager::onFile(const std::string &base, const std::string &file) {
	_base_dir = base;
}


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

		const std::string &model = attr["model"];
		if (model.size() == 0)
			throw_ex(("animation.model was not set"));

		long tw = atol(attr["tile_width"].c_str());
		long th = atol(attr["tile_height"].c_str());
		long sz = atol(attr["size"].c_str());
		if (tw == 0) tw = _tw;
		if (th == 0) th = _th;
		if (sz != 0) tw = th = sz;

		sdlx::Surface *s = NULL;
		sdlx::CollisionMap *cmap = NULL;
		bool real_load = !attr["persistent"].empty();
		GET_CONFIG_VALUE("engine.preload-all-resources", bool , preload_all, true);

		real_load |= preload_all;
		std::string &tile = attr["tile"];
		if (_base_dir.empty())
			throw_ex(("base directory was not defined (multiply resources tag ? invalid resource structure?)"));
		
		if (_surfaces.find(tile) == _surfaces.end()) {
			TRY { 		
				if (real_load) {
					const std::string fname = _base_dir + "/tiles/" + tile;
					s = new sdlx::Surface;
					s->loadImage(fname);
					s->convertAlpha();
			
					cmap = new sdlx::CollisionMap;
					cmap->init(s, sdlx::CollisionMap::OnlyOpaque);
			
					s->convertToHardware();
					LOG_DEBUG(("loaded animation '%s' from '%s'", id.c_str(), fname.c_str()));
				}
			
				_surfaces[tile] = s;
				s = NULL;
			
				_cmaps[tile] = cmap;
				cmap = NULL;
			
			} CATCH("animation", { delete s; s = NULL; delete cmap; cmap = NULL; throw; });
		//	
		} else { 
			LOG_DEBUG(("tile '%s' was already loaded, skipped.", tile.c_str()));
		}
	
		_animations[id] = new Animation(model, _base_dir, tile, tw, th);
	
	} else if (name == "animation-model") {
		const std::string & id = attr["id"];
		if (id.size() == 0) 
			throw_ex(("animation model must have id"));
		
		float speed = atof(attr["speed"].c_str());
		if (speed == 0)
			throw_ex(("animation model must have default speed"));
		
		_am = new AnimationModel(speed);
		_am_id = id;		
	} else if (name == "pose") {
		_pose_id = attr["id"];
		if (_pose_id.size() == 0) 
			throw_ex(("pose must have id"));
		float speed = atof(attr["speed"].c_str());
		if (speed == 0)
			speed = _am->default_speed;

		int z = (!attr["z"].empty())?atoi(attr["z"].c_str()) : -1001;
		const std::string &sound = attr["sound"];
		_pose = new Pose(speed, z, sound);
		if (!sound.empty())
			Mixer->loadSample(sound);
	} else if (name == "object") {
		const std::string classname = attr["class"];
		if (classname.size() == 0)
			throw_ex(("tag 'object' must provide its classname id."));
		ObjectMap::iterator object; 
		if ((object = _objects.find(classname)) == _objects.end()) {
			throw_ex(("class '%s' was not registered. ", classname.c_str()));
		}
		LOG_DEBUG(("setting up class '%s'", classname.c_str()));
	
		if (attr.find("parent") != attr.end())  {
			ObjectMap::iterator parent; 
			if ((parent = _objects.find(attr["parent"])) == _objects.end()) {
				throw_ex(("class '%s' declared as parent of '%s' was not registered. skipped.", attr["parent"].c_str(), classname.c_str()));
			}
			object->second->inheritParameters(parent->second);
		}
	
		for (Attrs::iterator i = attr.begin(); i != attr.end(); ++i) {
			const std::string &name = i->first;
			const std::string &value = i->second;
			if (name == "speed") {
				object->second->speed = atol(value.c_str());
			} else if (name == "mass") {
				object->second->mass = atof(value.c_str());
			} else if (name == "ttl") {
				object->second->ttl = atof(value.c_str());
			} else if (name == "piercing") {
				object->second->piercing = (value[0] == 't' || value[0] == '1' || value[0] == 'y');
			} else if (name == "pierceable") {
				object->second->pierceable = (value[0] == 't' || value[0] == '1' || value[0] == 'y');
			} else if (name == "hp") {
				object->second->max_hp = object->second->hp = atol(value.c_str());
			} else if (name == "impassability") {
				object->second->impassability = atof(value.c_str());
			} else if (name == "fadeout_time") {
				object->second->fadeout_time = atof(value.c_str());
			} else if (name == "z") {
				object->second->setZ(atoi(value.c_str()));
			} else if (name != "class" && name != "parent") 
				LOG_WARN(("attr '%s' is not supported", name.c_str()));
		}
		LOG_DEBUG(("%s", object->second->dump().c_str()));
	} else if (name == "alias") {
		std::string name = attr["name"];
		std::string classname = attr["class"];
		if (name.empty() || classname.empty())
			throw_ex(("alias must have both 'name' and 'class' attributes"));
		createAlias(name, classname);
	} else if (name == "sound") {
		std::string file = attr["file"];
		if (file.empty())
			throw_ex(("sound.file MUST not be empty."));
		TRY {
			Mixer->loadSample(file, attr["class"]);
		} CATCH("loadSample", {});
	} else LOG_WARN(("unhandled tag: %s", name.c_str()));
	NotifyingXMLParser::start(name, attr);
}

void IResourceManager::end(const std::string &name) {
	mrt::trim(_data);
	if (name == "pose") {
		LOG_DEBUG(("pose frames: %s", _data.c_str()));
		std::vector<std::string> frames;
		mrt::split(frames, _data, ",");
		
		for(size_t i = 0; i < frames.size(); ++i) {
			//LOG_DEBUG(("%d: %s", i, frames[i].c_str()));
			mrt::trim(frames[i]);
			unsigned int frame = atoi(frames[i].c_str());
			//LOG_DEBUG(("%d: %d", i, frame));
			_pose->frames.push_back(frame);
		}
		_am->addPose(_pose_id, _pose);
		_pose = NULL;
	} else if (name == "animation-model") {
		delete _animation_models[_am_id];
		_animation_models[_am_id] = _am;
		_am = NULL;
		LOG_DEBUG(("added animation model '%s'", _am_id.c_str()));
	} else if (name == "resources") {
		_base_dir.clear();
	}
	NotifyingXMLParser::end(name);
	_data.clear();
}
void IResourceManager::charData(const std::string &data) {
	_data += data;
}

IResourceManager::IResourceManager() : _am(0) {
}

const bool IResourceManager::hasAnimation(const std::string &id) const {
	return _animations.find(id) != _animations.end();
}

Animation *IResourceManager::getAnimation(const std::string &id) {
	AnimationMap::iterator i = _animations.find(id);
	if (i == _animations.end()) 
		throw_ex(("could not find animation with id '%s'", id.c_str()));
	return i->second;
}

const Animation *IResourceManager::getAnimation(const std::string &id) const {
	AnimationMap::const_iterator i = _animations.find(id);
	if (i == _animations.end()) 
		throw_ex(("could not find animation with id '%s'", id.c_str()));
	return i->second;
}

AnimationModel *IResourceManager::getAnimationModel(const std::string &id) {
	AnimationModelMap::iterator i = _animation_models.find(id);
	if (i == _animation_models.end()) 
		throw_ex(("could not find animation with id '%s'", id.c_str()));
	return i->second;
}

const sdlx::Surface *IResourceManager::getSurface(const std::string &id) const  {
	SurfaceMap::const_iterator i = _surfaces.find(id);
	if (i == _surfaces.end()) 
		throw_ex(("could not find surface with id '%s'", id.c_str()));
	return i->second;
}

const sdlx::Surface *IResourceManager::loadSurface(const std::string &id) {
	SurfaceMap::iterator i = _surfaces.find(id);
	if (i != _surfaces.end() && i->second != NULL)
		return i->second;
	
	const std::string fname = Finder->find("tiles/" + id);
	sdlx::Surface *s = NULL;
		TRY {
			s = new sdlx::Surface;
			s->loadImage(fname);
			s->convertAlpha();
			s->convertToHardware();
			LOG_DEBUG(("loaded surface '%s' from '%s'", id.c_str(), fname.c_str()));
			_surfaces[id] = s;
		} CATCH("loading surface", { delete s; throw; });
	return s;
}

const sdlx::Font *IResourceManager::loadFont(const std::string &name, const bool alpha) {
	std::pair<std::string, bool> id(name, alpha);
	FontMap::iterator i = _fonts.find(id);
	if (i != _fonts.end() && i->second != NULL)
		return i->second;
	
	const std::string fname = Finder->find("font/" + name + ".png");
	sdlx::Font *f = NULL;
		TRY {
			f = new sdlx::Font;
			f->load(fname, sdlx::Font::Ascii, alpha);
			LOG_DEBUG(("loaded font '%s' from '%s'", name.c_str(), fname.c_str()));
			_fonts[id] = f;
		} CATCH("loading font", { delete f; throw; });
	return f;
}


const sdlx::CollisionMap *IResourceManager::getCollisionMap(const std::string &id) const  {
	CollisionMap::const_iterator i = _cmaps.find(id);
	if (i == _cmaps.end()) 
		throw_ex(("could not find collision map with id '%s'", id.c_str()));
	return i->second;
}


void IResourceManager::init(const std::vector<std::pair<std::string, std::string> > &fname) {
	parseFiles(fname);
}

void IResourceManager::clear() {
	LOG_DEBUG(("freeing resources"));
	std::for_each(_animations.begin(), _animations.end(), delete_ptr2<AnimationMap::value_type>());
	_animations.clear();
	std::for_each(_animation_models.begin(), _animation_models.end(), delete_ptr2<AnimationModelMap::value_type>());
	_animation_models.clear();
	std::for_each(_surfaces.begin(), _surfaces.end(), delete_ptr2<SurfaceMap::value_type>());
	_surfaces.clear();
	std::for_each(_cmaps.begin(), _cmaps.end(), delete_ptr2<CollisionMap::value_type>());
	_cmaps.clear();
	std::for_each(_fonts.begin(), _fonts.end(), delete_ptr2<FontMap::value_type>());
	_fonts.clear();
	std::for_each(_objects.begin(), _objects.end(), delete_ptr2<ObjectMap::value_type>());
	_objects.clear();

	_am = NULL;
}

IResourceManager::~IResourceManager() {
}

void IResourceManager::registerObject(const std::string &classname, Object *o) {
	Variants vars;
	vars.parse(classname);
	if (!vars.empty())
		throw_ex(("registering object with variants ('%s') is prohibited", classname.c_str()));
	
	assert(!classname.empty());
	*const_cast<std::string *>(&o->registered_name) = classname;
	assert(!o->registered_name.empty());
	
	delete _objects[classname];
	_objects[classname] = o;
	//LOG_DEBUG(("classname %s registered at %p", classname.c_str(), (void*)o));
}

void IResourceManager::createAlias(const std::string &name, const std::string &_classname) {
	Variants vars;
	vars.parse(name);
	if (!vars.empty())
		throw_ex(("registering object with variants ('%s') is prohibited", name.c_str()));

	std::string classname = vars.parse(_classname);

	LOG_DEBUG(("creating alias '%s' -> '%s' (variants: '%s')", name.c_str(), classname.c_str(), vars.dump().c_str()));
	ObjectMap::const_iterator i = _objects.find(classname);

	if (i == _objects.end())
		throw_ex(("object %s was not registered", classname.c_str()));

	if (_objects.find(name) != _objects.end())
		throw_ex(("attempt to create alias with duplicate name ('%s')", name.c_str()));

	Object * r = i->second->clone();
	if (r == NULL)
		throw_ex(("%s->clone(\"\") returns NULL", classname.c_str()));

	*const_cast<std::string *>(&r->registered_name) = name;

	r->updateVariants(vars);
	_objects[name] = r;
}

Object *IResourceManager::createObject(const std::string &_classname) const {
	Variants vars;
	std::string classname = vars.parse(_classname);
	
	ObjectMap::const_iterator i = _objects.find(classname);
	if (i == _objects.end())
		throw_ex(("classname '%s' was not registered", classname.c_str()));
	Object * r = i->second->clone();

	assert(!r->registered_name.empty());//this usually happens if you constructed object with default ctor from clone() method. check it! 
										//clone should look like { return new Object(*this); }
	if (r == NULL)
		throw_ex(("%s->clone() returns NULL", classname.c_str()));

	r->updateVariants(vars);

	return r;
}


Object *IResourceManager::createObject(const std::string &classname, const std::string &animation) const {
	Object *r = createObject(classname);
	
	r->init(animation);
	//LOG_DEBUG(("base: %s", i->second->dump().c_str()));
	//LOG_DEBUG(("clone: %s", r->dump().c_str()));
	r->animation = animation;
	return r;
}

const Object *IResourceManager::getClass(const std::string &classname) const {
	ObjectMap::const_iterator i = _objects.find(classname);
	if (i == _objects.end())
		throw_ex(("classname '%s' was not registered", classname.c_str()));
	return i->second;	
}

void IResourceManager::checkSurface(const std::string &animation, const sdlx::Surface *& surface_ptr, const sdlx::CollisionMap *& cmap_ptr) {
	if (surface_ptr != NULL && cmap_ptr != NULL) 
		return;

	const Animation * a = getAnimation(animation);
	
	sdlx::Surface *s = _surfaces[a->surface];
	sdlx::CollisionMap *cmap = _cmaps[a->surface];

	const std::string fname = a->base_dir + "/tiles/" + a->surface;
	if (s == NULL) {
		TRY {
			s = new sdlx::Surface;
			s->loadImage(fname);
			s->convertAlpha();
			s->convertToHardware();
			GET_CONFIG_VALUE("engine.strip-alpha-from-object-tiles", bool, strip_alpha, false);
			if (strip_alpha) {
				Uint8 r,g,b,a;
				for(int y = 0; y < s->getHeight(); ++y) 
					for(int x = 0; x < s->getWidth(); ++x) {
						s->getRGBA(s->getPixel(x, y), r, g, b, a);
						if (a != 255)
							s->putPixel(x, y, s->mapRGBA(0,0,0,0));
					}
			}

			LOG_DEBUG(("loaded animation '%s' from '%s'", animation.c_str(), fname.c_str()));
			_surfaces[a->surface] = s;
		} CATCH("loading surface", { delete s; throw; });
	}
	surface_ptr = s;
	
	if (cmap == NULL) {			
		cmap = new sdlx::CollisionMap;
		cmap->init(s, sdlx::CollisionMap::OnlyOpaque);
		_cmaps[a->surface] = cmap;
	}
	cmap_ptr = cmap;
}
